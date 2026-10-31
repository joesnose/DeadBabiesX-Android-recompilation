#include "lan.h"
#include "xengine.h"
#include "game.h"
#include "results_presentation.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <array>
#include <vector>
#include <memory>
#include <cstring>
#include <stdexcept>
namespace {
constexpr uint32_t Magic=0x4442584c,Version=0x00010501;
constexpr uint16_t Port=39828;
enum Type {Hello=1,Start=2,Input=3,Frame=4,Error=5};
using Bytes=std::vector<uint8_t>;
void u16(Bytes&b,uint16_t v){b.push_back(v>>8);b.push_back(v);}
void u32(Bytes&b,uint32_t v){u16(b,v>>16);u16(b,v);}
void u64(Bytes&b,uint64_t v){u32(b,v>>32);u32(b,v);}
struct Reader {const Bytes&b;size_t p=0;uint8_t byte(){if(p>=b.size())throw std::runtime_error("Invalid LAN packet");return b[p++];}uint16_t word(){uint16_t a=byte();return (a<<8)|byte();}uint32_t dword(){uint32_t a=word();return (a<<16)|word();}uint64_t qword(){uint64_t a=dword();return (a<<32)|dword();}void end(){if(p!=b.size())throw std::runtime_error("Invalid LAN packet length");}};
struct Controls {int16_t axis[4]{};uint16_t buttons=0;uint8_t analog=0;};
Controls capture(const XBGAMEPAD&p){Controls c;float a[]={p.fX1,p.fY1,p.fX2,p.fY2};for(int i=0;i<4;i++)c.axis[i]=std::lround(std::clamp(a[i],-1.f,1.f)*32767);c.buttons=p.wButtons&0xff;for(int i=0;i<8;i++)if(p.bAnalogButtons[i]>75)c.analog|=1u<<i;return c;}
void write(Bytes&b,const Controls&c){for(int16_t a:c.axis)u16(b,uint16_t(a));u16(b,c.buttons);b.push_back(c.analog);}
Controls read(Reader&r){Controls c;for(auto&a:c.axis)a=int16_t(r.word());c.buttons=r.word();c.analog=r.byte();if(c.buttons&~0xff)throw std::runtime_error("Invalid LAN controls");return c;}
struct Session {
 std::atomic<bool> cancel{false},ready{false},active{false},start{false};
 std::thread worker;std::mutex lifecycle;std::mutex mutex;std::string status="Preparing room...";LanConfig config;
 std::vector<int> peers;int listener=-1;bool host=false;uint32_t frame=0;bool paused=false,loading=true;int menuMode=0,selection=0;LanAction action=LanAction::None;Controls previous[4]{};
 void joinWorker(){std::lock_guard<std::mutex>l(lifecycle);if(worker.joinable())worker.join();}
 ~Session(){cancel=true;joinWorker();for(int s:peers)close(s);if(listener>=0)close(listener);}
 void set(const std::string&s){std::lock_guard<std::mutex>l(mutex);status=s;}
 void fail(const std::string&s){set(s);cancel=true;}
};
std::atomic<bool>activeFlag{false};
std::mutex globalMutex;std::shared_ptr<Session>current;
std::shared_ptr<Session> get(){std::lock_guard<std::mutex>l(globalMutex);return current;}
uint64_t clockMs(){return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}
void setup(int s){int yes=1;setsockopt(s,IPPROTO_TCP,TCP_NODELAY,&yes,sizeof(yes));fcntl(s,F_SETFL,fcntl(s,F_GETFL)|O_NONBLOCK);}
void transfer(Session&s,int fd,uint8_t*p,size_t n,bool sending,uint64_t deadline,void(*pump)()){
 while(n){if(s.cancel)throw std::runtime_error("LAN game cancelled");if(clockMs()>deadline)throw std::runtime_error("LAN connection timed out. Keep all phones in the game.");if(pump)pump();
  pollfd poller{fd,short(sending?POLLOUT:POLLIN),0};int rc=poll(&poller,1,15);if(rc<0&&errno==EINTR)continue;if(rc<0)throw std::runtime_error("LAN socket error");if(!rc)continue;
  if(poller.revents&(POLLERR|POLLNVAL))throw std::runtime_error("LAN connection lost");
  ssize_t count=sending?send(fd,p,n,MSG_NOSIGNAL):recv(fd,p,n,0);
  if(count<0&&(errno==EAGAIN||errno==EWOULDBLOCK||errno==EINTR))continue;
  if(count<=0)throw std::runtime_error("A player disconnected from the LAN game");p+=count;n-=size_t(count);
 }
}
void sendPacket(Session&s,int fd,Type t,const Bytes&data,uint64_t deadline,void(*pump)()=nullptr){Bytes b;u32(b,Magic);u16(b,t);u16(b,data.size());b.insert(b.end(),data.begin(),data.end());transfer(s,fd,b.data(),b.size(),true,deadline,pump);}
Bytes receive(Session&s,int fd,Type t,uint64_t deadline,void(*pump)()=nullptr){Bytes h(8);transfer(s,fd,h.data(),h.size(),false,deadline,pump);Reader r{h};if(r.dword()!=Magic)throw std::runtime_error("Incompatible LAN game. Install the same APK on every phone.");auto kind=r.word(),n=r.word();if(n>128)throw std::runtime_error("Invalid LAN packet size");Bytes b(n);transfer(s,fd,b.data(),n,false,deadline,pump);if(kind==Error){for(auto&c:b)if(c<32||c>126)c=63;throw std::runtime_error(std::string(b.begin(),b.end()));}if(kind!=t)throw std::runtime_error("Incompatible LAN message type");return b;}
void handshake(Session&s,int fd){auto b=receive(s,fd,Hello,clockMs()+2500);Reader r{b};if(r.dword()!=Version)throw std::runtime_error("LAN version mismatch. Use the same APK on all phones.");r.end();}
std::shared_ptr<Session> make(bool host){auto old=get();if(old&&old->active)return {};LanStop();auto s=std::make_shared<Session>();s->host=host;{std::lock_guard<std::mutex>l(globalMutex);current=s;}return s;}
}
void LanHost(bool mega){auto s=make(true);if(!s)return;s->config.mega=mega;s->worker=std::thread([s]{try{
 s->listener=socket(AF_INET,SOCK_STREAM,0);if(s->listener<0)throw std::runtime_error("Cannot open LAN host socket");setup(s->listener);int yes=1;setsockopt(s->listener,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_port=htons(Port);addr.sin_addr.s_addr=INADDR_ANY;
 if(bind(s->listener,(sockaddr*)&addr,sizeof(addr))||listen(s->listener,3))throw std::runtime_error("Cannot host LAN room: port 39828 is already in use");
 while(!s->cancel){s->set("HOST: "+std::to_string(s->peers.size()+1)+" / 4 players. Share your Wi-Fi IP. Tap Start once everyone joins.");
  for(auto it=s->peers.begin();it!=s->peers.end();){char c;int n=recv(*it,&c,1,MSG_PEEK|MSG_DONTWAIT);if(n==0||(n<0&&errno!=EAGAIN&&errno!=EWOULDBLOCK&&errno!=EINTR)){close(*it);it=s->peers.erase(it);}else ++it;}
  if(s->start){if(!s->peers.empty())break;s->start=false;}pollfd p{s->listener,POLLIN,0};if(poll(&p,1,40)<=0)continue;int fd=accept(s->listener,nullptr,nullptr);if(fd<0)continue;setup(fd);
  if(s->peers.size()>=3){close(fd);continue;}try{handshake(*s,fd);s->peers.push_back(fd);}catch(...){close(fd);}
 }
 if(s->cancel)return;close(s->listener);s->listener=-1;s->config.players=s->peers.size()+1;s->config.slot=0;s->config.seed=uint32_t(clockMs())^0xa7c3e529u;
 for(size_t i=0;i<s->peers.size();i++){Bytes b;u32(b,Version);u32(b,s->config.seed);b.push_back(i+1);b.push_back(s->config.players);b.push_back(s->config.mega);sendPacket(*s,s->peers[i],Start,b,clockMs()+3000);}
 s->set("Starting LAN game...");s->ready=true;
 }catch(const std::exception&e){s->fail(e.what());}});}
void LanJoin(const std::string&ip){auto s=make(false);if(!s)return;s->set("Connecting to "+ip+"...");s->worker=std::thread([s,ip]{try{
 sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_port=htons(Port);if(inet_pton(AF_INET,ip.c_str(),&addr.sin_addr)!=1)throw std::runtime_error("Enter the host's Wi-Fi IPv4 address");
 uint32_t a=ntohl(addr.sin_addr.s_addr);if(!((a>>24)==10||(a>>20)==0xac1||(a>>16)==0xc0a8||(a>>24)==127||(a>>16)==0xa9fe))throw std::runtime_error("Use a local Wi-Fi IPv4 address");
 int fd=socket(AF_INET,SOCK_STREAM,0);if(fd<0)throw std::runtime_error("Cannot open LAN socket");s->peers.push_back(fd);setup(fd);
 int rc=connect(fd,(sockaddr*)&addr,sizeof(addr));if(rc<0&&errno!=EINPROGRESS)throw std::runtime_error("Cannot connect to host. Check Wi-Fi and IP address.");
 uint64_t until=clockMs()+6000;while(rc<0){if(s->cancel) return;pollfd p{fd,POLLOUT,0};if(poll(&p,1,50)>0){int error=0;socklen_t n=sizeof(error);getsockopt(fd,SOL_SOCKET,SO_ERROR,&error,&n);if(error)throw std::runtime_error("Host is unreachable. Check the IP and Wi-Fi network.");break;}if(clockMs()>until)throw std::runtime_error("Connection timed out. Check host IP and Wi-Fi.");}
 Bytes hello;u32(hello,Version);sendPacket(*s,fd,Hello,hello,clockMs()+2000);s->set("Connected. Waiting for the host to start...");auto b=receive(*s,fd,Start,clockMs()+300000);Reader r{b};if(r.dword()!=Version)throw std::runtime_error("LAN version mismatch");s->config.seed=r.dword();s->config.slot=r.byte();s->config.players=r.byte();s->config.mega=r.byte()!=0;r.end();if(s->config.players<2||s->config.players>4||s->config.slot<1||s->config.slot>=s->config.players)throw std::runtime_error("Invalid LAN player assignment");s->ready=true;
 }catch(const std::exception&e){s->fail(e.what());}});}
void LanStart(){auto s=get();if(s&&s->host)s->start=true;}
void LanCancel(){auto s=get();if(s){s->fail("LAN game cancelled");if(s->active)for(int fd:s->peers)shutdown(fd,SHUT_RDWR);}}
void LanStop(){activeFlag=false;std::shared_ptr<Session>s;{std::lock_guard<std::mutex>l(globalMutex);s=std::move(current);}if(s){s->cancel=true;s->joinWorker();}}
std::string LanStatus(){auto s=get();if(!s)return "No LAN room";if(s->active&&!s->cancel)return "PLAYING";std::lock_guard<std::mutex>l(s->mutex);return s->status;}
bool LanTakeReady(LanConfig&c){auto s=get();if(s&&s->cancel&&!s->active){s->joinWorker();for(int fd:s->peers)close(fd);s->peers.clear();if(s->listener>=0){close(s->listener);s->listener=-1;}return false;}if(!s||!s->ready||s->cancel||s->active)return false;s->joinWorker();if(s->cancel)return false;c=s->config;s->active=true;activeFlag=true;return true;}
bool LanActive(){return activeFlag.load(std::memory_order_relaxed);}
uint32_t LanSeed(){auto s=get();return s?s->config.seed:1;}
int LanPlayerSlot(){auto s=get();return s?s->config.slot:0;}
bool LanExchange(XBGAMEPAD*pads,uint64_t hash,bool&paused,void(*pump)()){
 auto s=get();if(!s||!s->active||s->cancel)return false;try{
 uint64_t deadline=clockMs()+(s->loading?30000:7000);Controls controls[4]{};auto local=capture(pads[0]);
 if(s->host){controls[0]=local;for(size_t i=0;i<s->peers.size();i++){auto b=receive(*s,s->peers[i],Input,deadline,pump);Reader r{b};if(r.dword()!=s->frame)throw std::runtime_error("LAN frame mismatch");if(r.qword()!=hash)throw std::runtime_error(("LAN lost sync at frame "+std::to_string(s->frame)+". Install the same version on every phone."));controls[i+1]=read(r);r.end();}
  bool results=Game.m_ResultsReady&&NativeResultsCanContinue(Game.m_ResultOrder.size(),Game.m_ResultsTime/60.f);
  bool start=false,back=false,confirm=false,navigate=false;int direct=-1;
  for(int i=0;i<s->config.players;i++){
   const auto&in=controls[i];const auto&old=s->previous[i];uint16_t edge=in.buttons&~old.buttons;
   start|=(edge&16)!=0;back|=(edge&32)!=0;confirm|=(in.analog&1)&&!(old.analog&1);
   if(edge&64)direct=0;if(edge&128)direct=1;
   auto direction=[](const Controls&c){int mask=c.buttons&15;if(c.axis[0]>18000)mask|=8;if(c.axis[0]<-18000)mask|=4;if(c.axis[1]>18000)mask|=1;if(c.axis[1]<-18000)mask|=2;return mask;};
   navigate|=(direction(in)&~direction(old))!=0;back|=s->menuMode!=0&&(in.analog&2)&&!(old.analog&2);
  }
  bool opened=false;
  if(!results&&(start||back)){opened=!s->paused;s->paused=!s->paused;}
  int mode=results?2:s->paused?1:0;
  bool changed=mode!=s->menuMode;if(changed)s->selection=0;s->menuMode=mode;s->action=LanAction::None;
  if(mode){
   if(navigate&&!changed)s->selection=1-s->selection;
   if(direct>=0){s->selection=direct;confirm=true;}
   if(results&&back)s->action=LanAction::MainMenu;
   else if(!opened&&(confirm||(results&&start)))s->action=s->selection==0?(results?LanAction::PlayAgain:LanAction::None):LanAction::MainMenu;
   if(mode==1&&!opened&&confirm&&s->selection==0){s->paused=false;s->menuMode=0;}
  }
  Bytes frame;u32(frame,s->frame);u64(frame,hash);frame.push_back(s->menuMode);frame.push_back(s->selection);frame.push_back(int(s->action));for(int i=0;i<s->config.players;i++)write(frame,controls[i]);for(int fd:s->peers)sendPacket(*s,fd,Frame,frame,deadline,pump);
 }else{Bytes b;u32(b,s->frame);u64(b,hash);write(b,local);sendPacket(*s,s->peers[0],Input,b,deadline,pump);b=receive(*s,s->peers[0],Frame,deadline,pump);Reader r{b};if(r.dword()!=s->frame||r.qword()!=hash)throw std::runtime_error(("LAN lost sync at frame "+std::to_string(s->frame)+". Install the same version on every phone."));s->menuMode=r.byte();s->selection=r.byte();int action=r.byte();if(s->menuMode>2||s->selection>1||action>2)throw std::runtime_error("Invalid LAN menu state");s->action=LanAction(action);s->paused=s->menuMode==1;for(int i=0;i<s->config.players;i++)controls[i]=read(r);r.end();}
 for(int i=0;i<4;i++){auto&out=pads[i];auto&in=controls[i];out=XBGAMEPAD{};if(i>=s->config.players)continue;out.hDevice=(void*)(uintptr_t)(i+1);out.wButtons=in.buttons&15;out.wPressedButtons=(in.buttons&~s->previous[i].buttons)&15;out.wLastButtons=out.wButtons;float* axes[]={&out.fX1,&out.fY1,&out.fX2,&out.fY2};for(int k=0;k<4;k++)*axes[k]=std::clamp(in.axis[k]/32767.f,-1.f,1.f);for(int k=0;k<8;k++){bool on=in.analog&(1u<<k);out.bAnalogButtons[k]=on?255:0;out.bPressedAnalogButtons[k]=on&&!(s->previous[i].analog&(1u<<k));out.bLastAnalogButtons[k]=on;}s->previous[i]=in;}
 paused=s->paused;s->loading=false;s->frame++;return true;
 }catch(const std::exception&e){
 if(FILE*f=fopen(getenv("DBX_LAN_DIAGNOSTICS")?getenv("DBX_LAN_DIAGNOSTICS"):"lan-diagnostics.txt","a")){fprintf(f,"frame=%u slot=%d hash=%016llx error=%s\n",s->frame,s->config.slot,(unsigned long long)hash,e.what());fclose(f);}
 if(s->host){std::string text=e.what();if(text.size()>120)text.resize(120);Bytes msg(text.begin(),text.end());for(int fd:s->peers)try{sendPacket(*s,fd,Error,msg,clockMs()+100);}catch(...){};}
 s->fail(e.what());return false;}}

bool LanLobbyOpen(){auto s=get();return s&&!s->cancel&&!s->active;}

LanAction LanTakeAction(){auto s=get();if(!s)return LanAction::None;auto a=s->action;s->action=LanAction::None;if(a==LanAction::PlayAgain){s->config.seed=s->config.seed*1664525u+1013904223u;s->paused=false;s->menuMode=0;s->selection=0;s->loading=true;}return a;}
int LanMenuMode(){auto s=get();return s&&s->active?s->menuMode:0;}
int LanMenuSelection(){auto s=get();return s?s->selection:0;}
