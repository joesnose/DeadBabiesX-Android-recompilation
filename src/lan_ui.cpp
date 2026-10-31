#include "lan.h"
#include "vita_lan_ui.h"
#include "vita_platform.h"
#include "xengine.h"
#include <array>
#include <cstdio>
namespace {int mode=0,selection=0,octet=3;bool mega=false;std::array<int,4>ip{{192,168,1,100}};int repeatDir=0;Uint64 nextRepeat=0;
void change(int dir){if(mode==1)selection=(selection+dir+3)%3;else if(mode==3)ip[octet]=(ip[octet]+dir+256)%256;}
void close(){mode=0;repeatDir=0;LanCancel();LanStop();XEngine->ClearInputEdges();}
void accept(){if(mode==1){if(selection==0){LanHost(mega);mode=2;}else if(selection==1)mode=3;else close();}else if(mode==2){auto text=LanStatus();if(text.rfind("HOST:",0)==0&&text.rfind("HOST: 1 ",0)!=0)LanStart();}else if(mode==3){char text[32];snprintf(text,sizeof(text),"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);if(FILE*f=fopen("lan-host-ip.txt","w")){fputs(text,f);fclose(f);}LanJoin(text);mode=4;}}
}
void NativeShowLanSetup(bool m){mega=m;mode=1;selection=0;repeatDir=0;if(FILE*f=fopen("lan-host-ip.txt","r")){int v[4];if(fscanf(f,"%d.%d.%d.%d",v,v+1,v+2,v+3)==4)for(int i=0;i<4;i++)if(v[i]>=0&&v[i]<=255)ip[i]=v[i];fclose(f);}}
bool VitaLanUiActive(){return mode!=0;}
void VitaLanUiClose(){mode=0;repeatDir=0;}
bool VitaLanUiEvent(const SDL_Event&e){if(!mode)return false;
 if(e.type==SDL_CONTROLLERBUTTONDOWN){int b=e.cbutton.button;
  if(b==SDL_CONTROLLER_BUTTON_B||b==SDL_CONTROLLER_BUTTON_BACK){close();return true;}
  if(b==SDL_CONTROLLER_BUTTON_A||b==SDL_CONTROLLER_BUTTON_START){accept();return true;}
  int dir=b==SDL_CONTROLLER_BUTTON_DPAD_UP?-1:b==SDL_CONTROLLER_BUTTON_DPAD_DOWN?1:0;if(dir){change(mode==3?-dir:dir);repeatDir=mode==3?-dir:dir;nextRepeat=SDL_GetTicks64()+350;}
  if(mode==3){if(b==SDL_CONTROLLER_BUTTON_DPAD_LEFT)octet=(octet+3)%4;if(b==SDL_CONTROLLER_BUTTON_DPAD_RIGHT)octet=(octet+1)%4;if(b==SDL_CONTROLLER_BUTTON_X)ip[octet]=(ip[octet]+10)%256;if(b==SDL_CONTROLLER_BUTTON_Y)ip[octet]=(ip[octet]+246)%256;}return true;
 }
 if(e.type==SDL_CONTROLLERBUTTONUP){repeatDir=0;return true;}
 if(e.type==SDL_KEYDOWN){if(e.key.keysym.sym==SDLK_ESCAPE){close();return true;}if(e.key.keysym.sym==SDLK_RETURN){accept();return true;}if(e.key.keysym.sym==SDLK_UP)change(-1);if(e.key.keysym.sym==SDLK_DOWN)change(1);return true;}
 return e.type==SDL_FINGERDOWN||e.type==SDL_FINGERUP||e.type==SDL_FINGERMOTION;
}
void VitaLanUiDraw(){if(repeatDir&&SDL_GetTicks64()>=nextRepeat){change(repeatDir);nextRepeat=SDL_GetTicks64()+70;}
 XEngine->SetCameraPos(0,0);XEngine->FillRect(-20,-20,680,520,.03f,.022f,.015f,1);XEngine->DrawUiText("LAN MULTIPLAYER",320,45,25,255,230,180,true);XEngine->DrawUiText("SAME WI-FI - SAME GAME VERSION - UP TO FOUR PLAYERS",320,92,10,185,165,140,true);
 if(mode==1){const char*items[]={"HOST GAME","JOIN GAME","BACK"};for(int i=0;i<3;i++){XEngine->FillRect(130,165+i*55,380,42,i==selection?.3f:.1f,.10f,.04f,1);XEngine->DrawUiText(items[i],320,174+i*55,18,255,225,185,true);}}
 else if(mode==3){XEngine->DrawUiText("HOST WI-FI IP ADDRESS",320,150,16,240,210,170,true);for(int i=0;i<4;i++){char text[8];snprintf(text,sizeof(text),"%d",ip[i]);XEngine->FillRect(100+i*115,210,95,50,i==octet?.32f:.10f,.10f,.04f,1);XEngine->DrawUiText(text,147+i*115,218,22,255,230,180,true);}XEngine->DrawUiText("LEFT / RIGHT: OCTET   UP / DOWN: VALUE",320,310,11,210,190,160,true);XEngine->DrawUiText("SQUARE: +10   TRIANGLE: -10",320,340,11,210,190,160,true);}
 else {std::string status=LanStatus();if(status.size()>88)status.resize(88);XEngine->DrawUiText(status.c_str(),320,225,10,235,210,170,true);if(mode==2){auto addr=std::string("YOUR WI-FI IP: ")+VitaWifiAddress();XEngine->DrawUiText(addr.c_str(),320,165,14,255,230,185,true);XEngine->DrawUiText("CROSS / START: BEGIN ONCE EVERYONE JOINS",320,290,11,220,195,160,true);}}
 XEngine->DrawUiText(mode==3?"CROSS / START: CONNECT   CIRCLE: CANCEL":"D-PAD: SELECT   CROSS: CONFIRM   CIRCLE: CANCEL",320,435,10,205,185,155,true);
}
