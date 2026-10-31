#if !defined(__ANDROID__) && !defined(__vita__)
#include "lan.h"
#include "game.h"
#include "frontend.h"
#include <SDL.h>
#include <cstdio>
#include <string>
extern void ResetNativeRound();
int NativeLanIntegrationTest(int argc,char**argv){
 if(argc<4)return 2;bool host=std::string(argv[1])=="--lan-host";int expected=atoi(argv[2]),frames=atoi(argv[3]);std::string mode=argc>4?argv[4]:"normal";
 NativeSetHeadlessInputTest(true);SDL_Init(SDL_INIT_GAMECONTROLLER|SDL_INIT_EVENTS);
 if(host)LanHost(true);else LanJoin("127.0.0.1");
 LanConfig config;uint64_t until=SDL_GetTicks64()+12000;
 while(!LanTakeReady(config)){
  if(mode.rfind("reject:",0)==0&&LanStatus().find(mode.substr(7))!=std::string::npos){fprintf(stderr,"PASS: rejected %s\n",LanStatus().c_str());LanStop();return 0;}
  if(host&&LanStatus().find("HOST: "+std::to_string(expected)+" ")==0)LanStart();
  if(SDL_GetTicks64()>until){fprintf(stderr,"LOBBY FAIL: %s\n",LanStatus().c_str());LanStop();return 1;}SDL_Delay(2);
 }
 fprintf(stderr,"READY slot=%d players=%d mega=%d seed=%u\n",config.slot,config.players,config.mega,config.seed);
 XEngine->m_Ticks=0;XEngine->SetGameSpeed(1);SetStartingPlayers(config.players);SetMegaWeapons(config.mega);Game.Init();NativeSetTouchScene(true,false);
 if(!NativeLanLatchSelfTest()){fprintf(stderr,"LAN touch pulse latch failed\n");LanStop();return 1;}
 fprintf(stderr,"PASS: touch pause/results taps survive network polling\n");
 bool pauseObserved=false,resumeObserved=false,fireObserved=false;int rematches=0;uint32_t firstSeed=LanSeed();
 for(int frame=0;frame<frames;frame++){
  auto&local=XEngine->GamePad[0];local=XBGAMEPAD{};local.hDevice=(void*)1;local.fX1=(frame%120<60?.65f:-.65f);local.fX2=.55f;local.fY2=.35f;local.bAnalogButtons[7]=(frame%80<40)?255:0;local.bAnalogButtons[0]=255;
  if(mode=="normal"&&frame%120==0)for(auto*p:Game.m_PlayerList){p->m_PlayerCondition=PC_INVINCIBLE;p->m_ConditionTime=300;}
  if(host&&(frame==80||frame==90))local.wButtons=16;
  if(mode=="results"&&host&&frame==700)local.wButtons=16;
  if(mode=="results"&&host&&frame==710)local.wButtons=128;
  if(mode=="pauseexit"&&host&&frame==90)local.wButtons=128;
  if(!host&&mode=="desync"&&frame==20)Game.m_PlayerList.front()->m_Score++;
  if(!host&&mode=="disconnect"&&frame==20){LanCancel();LanStop();fprintf(stderr,"INTENTIONAL DISCONNECT\n");return 0;}
  if(mode=="results"&&frame==120)for(auto*p:Game.m_PlayerList)p->m_Lives=-1;
  if(mode=="results")local.bAnalogButtons[0]=frame==550?255:0;
  bool paused=false;if(!LanExchange(XEngine->GamePad,LanGameHash(),paused,nullptr)){
   auto message=LanStatus();fprintf(stderr,"EXCHANGE END: %s\n",message.c_str());LanStop();
   bool intended=(mode=="desync"&&(message.find("sync")!=std::string::npos||message.find("disconnected")!=std::string::npos))||(mode=="disconnect"&&message.find("disconnected")!=std::string::npos);return intended?0:1;
  }
  if(frame==81&&paused&&!NativeLanMenuTouchSelfTest()){fprintf(stderr,"LAN pause touch targets failed\n");LanStop();return 1;}
  auto action=LanTakeAction();
  if(action==LanAction::PlayAgain){
   if(mode!="results"){fprintf(stderr,"Unexpected rematch action\n");LanStop();return 1;}
   rematches++;XEngine->m_Ticks=0;XEngine->SetGameSpeed(1);ResetNativeRound();
   if(!LanActive()||LanPlayerSlot()!=config.slot||LanSeed()==firstSeed||Game.m_NumPlayers!=config.players){LanStop();return 1;}
   fprintf(stderr,"RESULTS REMATCH SYNCHRONIZED: same connected players, fresh seed, reset round\n");continue;
  }
  if(action==LanAction::MainMenu){
   bool ok=(mode=="results"&&rematches==1)||(mode=="pauseexit"&&pauseObserved);
   fprintf(stderr,"PAUSE EXIT TO MAIN MENU: %s\n",ok?"PASS":"FAIL");LanStop();return ok?0:1;
  }
  if(frame%60==0)fprintf(stderr,"FRAME %d HASH %016llx\n",frame,(unsigned long long)LanGameHash());
  if(paused)pauseObserved=true;else{
   if(pauseObserved)resumeObserved=true;
   int ongoing=Game.Update();XEngine->m_Ticks+=XEngine->m_GameSpeed;
   fireObserved|=!Game.m_BulletList.empty()||!Game.m_PlasmaList.empty()||!Game.m_RocketList.empty()||!Game.m_FlameList.empty()||!Game.m_GrenadeList.empty();
   if(!ongoing){fprintf(stderr,"Round ended before session menu choice\n");LanStop();return 1;}
  }
 }
 fprintf(stderr,"LAN MATCH PASS slot=%d ticks=%.1f paused=%d resumed=%d projectiles=%d hash=%016llx\n",config.slot,XEngine->m_Ticks,pauseObserved,resumeObserved,fireObserved,(unsigned long long)LanGameHash());
 // Let the last broadcast reach every peer before normal socket shutdown.
 SDL_Delay(100);LanStop();SDL_Quit();return mode!="results"&&pauseObserved&&resumeObserved&&fireObserved?0:1;
}
#endif
