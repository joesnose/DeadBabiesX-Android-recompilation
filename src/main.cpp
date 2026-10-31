#include "frontend.h"
#include "game.h"
#include "lan.h"
#include "controller_map.h"
#include <exception>
#include <string>
extern void ResetNativeRound();
extern int NativeSelfTest();
extern int NativeAudioSelfTest();
extern void NativeStopAudio();
#include <SDL.h>
int main(int argc,char**argv) {
#ifndef __ANDROID__
 extern int NativeAndroidSelfTest();
 if(argc>1&&std::string(argv[1])=="--controller-map-test")return ControllerMapSelfTest();
 extern int NativeInputSelfTest();
 extern int NativeRelaunchSelfTest();
 extern int NativePauseMenuSelfTest();
 if(argc>1&&std::string(argv[1])=="--pause-menu-test")return NativePauseMenuSelfTest();
 if(argc>1&&std::string(argv[1])=="--relaunch-self-test")return NativeRelaunchSelfTest();
 extern int NativeGameplayControllerSelfTest();
 extern int NativeLanIntegrationTest(int,char**);
 if(argc>1&&(std::string(argv[1])=="--lan-host"||std::string(argv[1])=="--lan-client"))return NativeLanIntegrationTest(argc,argv);
 if(argc>1&&std::string(argv[1])=="--gameplay-controller-test")return NativeGameplayControllerSelfTest();
 if(argc>1&&std::string(argv[1])=="--input-self-test")return NativeInputSelfTest();
 if(argc>1&&std::string(argv[1])=="--android-self-test")return NativeAndroidSelfTest();
#endif
  if(argc>1 && std::string(argv[1])=="--self-test")return NativeSelfTest();
  try {
    XEngine->XInit();
    Frontend frontend;
    bool controllerScreenWasOpen=false;
    XEngine->PlayMusic("d:\\data\\music\\music3.wma");
    bool roundExists = false;
    if (getenv("DBX_STRESS")) {
      XEngine->StopAllSounds();
      SetMegaWeapons(frontend.MegaWeapons());
      Game.Init();
      roundExists = true;
      frontend.Play();
    }
    bool resetTestDone = false;
    while (XEngine->Running()) {
      LanConfig lanConfig;
      if(!frontend.playing&&LanTakeReady(lanConfig)){
        NativePrepareGameplayController(-1);SetStartingPlayers(lanConfig.players);SetMegaWeapons(lanConfig.mega);
        XEngine->m_Ticks=0;XEngine->SetGameSpeed(1);
        if(roundExists)ResetNativeRound();else{XEngine->StopAllSounds();Game.Init();roundExists=true;}
        frontend.Play();continue;
      }
      NativeSetTouchScene(frontend.playing, Game.m_ResultsReady);
      if (!frontend.playing) {
        if(ControllerMapActive()){controllerScreenWasOpen=true;ControllerMapDraw();XEngine->XUpdate();continue;}
        if(controllerScreenWasOpen){controllerScreenWasOpen=false;frontend.OpenTitle();continue;}
        if(LanLobbyOpen()){frontend.Draw();XEngine->XUpdate();continue;}
        MenuAction action = frontend.Update();
        if (action == MenuAction::Exit) {
          fprintf(stderr, "MENU exit\n");
          XEngine->RequestExit();
          continue;
        }
        if (action == MenuAction::Start) {
          fprintf(stderr, "MENU start\n");
          SetStartingPlayers(frontend.PlayerCount());
          SetMegaWeapons(frontend.MegaWeapons());
          if (roundExists)
            ResetNativeRound();
          else {
            XEngine->StopAllSounds();
            Game.Init();
            roundExists = true;
          }
          frontend.Play();
          continue;
        }
        if (action == MenuAction::ArtChanged && roundExists) {
          Game.LoadArt();
          auto *player = Game.m_PlayerList.empty() ? nullptr : Game.m_PlayerList.front();
          fprintf(stderr, "ART live reload complete; score=%d players=%d x=%.2f y=%.2f power=%d\n",
                  player ? player->m_Score : 0, Game.m_NumPlayers,
                  player ? player->m_PlayerPos.x : 0, player ? player->m_PlayerPos.y : 0,
                  player ? player->m_Weapon->m_PowerLevel : 0);
        }
        if (action == MenuAction::Resume) {
          fprintf(stderr, "MENU resume\n");
          frontend.Play();
          continue;
        }
        if (action == MenuAction::Title) {
          XEngine->SetAudioPaused(false);
          XEngine->StopAllSounds(false);
          XEngine->PlayMusic("d:\\data\\music\\music3.wma");
          frontend.OpenTitle();
        }
        if(ControllerMapActive()){controllerScreenWasOpen=true;ControllerMapDraw();}else frontend.Draw();
        XEngine->XUpdate();
        continue;
      }
      NativeSetTouchScene(frontend.playing, Game.m_ResultsReady);
      if(LanActive()){
        if(XEngine->TakeMenuBack())NativeLanRequestBack();
        XEngine->GamePad[0]=NativeLanReadLocalInput();
        bool networkPaused=false;
        if(!LanExchange(XEngine->GamePad,LanGameHash(),networkPaused,NativeLanPump)){
          std::string reason=LanStatus();LanStop();XEngine->StopAllSounds(false);XEngine->SetAudioPaused(false);XEngine->PlayMusic("music3.ogg");frontend.OpenTitle();
          SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,"LAN multiplayer",reason.c_str(),nullptr);continue;
        }
        LanAction sessionAction=LanTakeAction();
        if(sessionAction==LanAction::MainMenu){LanStop();XEngine->StopAllSounds(false);XEngine->SetAudioPaused(false);XEngine->PlayMusic("music3.ogg");frontend.OpenTitle();continue;}
        if(sessionAction==LanAction::PlayAgain){XEngine->m_Ticks=0;XEngine->SetGameSpeed(1);ResetNativeRound();frontend.Play();continue;}
        if(networkPaused){XEngine->SetAudioPaused(true);Game.Refresh();XEngine->XUpdate();continue;}
        XEngine->SetAudioPaused(false);
        bool ongoing=Game.Update()!=0;XEngine->m_Ticks+=XEngine->m_GameSpeed;
        if(!ongoing){LanStop();XEngine->StopAllSounds(false);XEngine->PlayMusic("music3.ogg");frontend.OpenTitle();}
        continue;
      }
      if (frontend.WantsPause()) {
        frontend.OpenPause();
        continue;
      }
      if (getenv("DBX_STRESS"))
        for (auto *player : Game.m_PlayerList) {
          player->m_PlayerCondition = PC_INVINCIBLE;
          player->m_ConditionTime = 300;
        }
      if (const char *reset = getenv("DBX_TEST_RESTART_AT")) {
        if (!resetTestDone && XEngine->m_Ticks >= atof(reset) * 60) {
          ResetNativeRound();
          resetTestDone = true;
          fprintf(stderr, "TEST round restart completed\n");
        }
      }
      if (getenv("DBX_TEST_PICKUPS"))
        XEngine->GamePad[0].bAnalogButtons[0] = 255;
      if (!Game.Update()) {
        XEngine->StopAllSounds(false);
        XEngine->PlayMusic("d:\\data\\music\\music3.wma");
        frontend.OpenTitle();
      }
    }
    XEngine->Shutdown();
    return 0;
  } catch (const std::exception &e) {
    XEngine->Shutdown();
    fprintf(stderr, "FATAL: %s\n", e.what());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Dead Babies X",e.what(),nullptr);
    return 1;
  }
}
