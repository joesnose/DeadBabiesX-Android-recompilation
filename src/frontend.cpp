#include "frontend.h"
#include <string>
#include "ui_style.h"
Frontend::Frontend() {
  const char *skin = NativeArtSkin();
  artMode = skin && std::string(skin) == "cartoon" ? 5 :
            skin && std::string(skin) == "celshaded" ? 4 :
            skin && std::string(skin) == "supersuperhd" ? 3 :
            skin && std::string(skin) == "superhd" ? 2 :
            skin && std::string(skin) == "hd" ? 1 : 0;
  NativeSetArtMode(artMode);
  megaWeapons = getenv("DBX_TEST_MEGA") != nullptr;
  backdrop = XEngine->LoadTexture("d:\\data\\backgrounds\\train_bg3.png");
  fprintf(stderr, "MENU title\n");
}
void Frontend::OpenPause() {
  playing = false;
  paused = true;
  selection = 0;
  heldDirection = 0;
  heldHorizontal = 0;
  inputArmed = false;
  XEngine->SetAudioPaused(true);
  XEngine->ClearInputEdges();
  fprintf(stderr, "MENU pause\n");
}
void Frontend::OpenTitle() {
  playing = false;
  paused = false;
  selection = 0;
  heldDirection = 0;
  heldHorizontal = 0;
  inputArmed = false;
  XEngine->ClearInputEdges();
  fprintf(stderr, "MENU title\n");
}
void Frontend::Play() {
  playing = true;
  paused = false;
  selection = 0;
  heldDirection = 0;
  heldHorizontal = 0;
  inputArmed = false;
  Game.m_Paused = false;
  XEngine->ClearInputEdges();
  XEngine->SetAudioPaused(false);
}
bool Frontend::WantsPause() {
  if (Game.m_ResultsReady || Game.m_NumPlayers <= 0) return false;
  if (XEngine->TakeMenuBack())
    return true;
  for (auto *player : Game.m_PlayerList)
    if (XEngine->GamePad[player->m_ControllerNum].wPressedButtons & 0x10)
      return true;
  return false;
}
MenuAction Frontend::Update() {
  bool confirm = false, back = XEngine->TakeMenuBack();
  int direction = 0, horizontal = 0, starterPad = -1;
  bool controlsHeld = false;
  for (int padIndex=0;padIndex<4;padIndex++) {
    const auto &pad=XEngine->GamePad[padIndex];
    if (!pad.hDevice)
      continue;
    controlsHeld = controlsHeld || (pad.wButtons & 0x3f) ||
                   pad.bAnalogButtons[0] > 75 || pad.bAnalogButtons[1] > 75;
    if ((pad.wPressedButtons & 1) || pad.fY1 > .55f || pad.fY2 > .55f)
      direction = -1;
    if ((pad.wPressedButtons & 2) || pad.fY1 < -.55f || pad.fY2 < -.55f)
      direction = 1;
    if ((pad.wPressedButtons & 4) || pad.fX1 < -.55f || pad.fX2 < -.55f)
      horizontal = -1;
    if ((pad.wPressedButtons & 8) || pad.fX1 > .55f || pad.fX2 > .55f)
      horizontal = 1;
    if(pad.bPressedAnalogButtons[0] || (pad.wPressedButtons & 0x10)){confirm=true;starterPad=padIndex;}
    back = back || pad.bPressedAnalogButtons[1] || (pad.wPressedButtons & 0x20);
  }
  if (back && paused)
    return MenuAction::Resume;
  if (!inputArmed) {
    // Aiming or holding Start when pausing must not navigate or confirm.
    if (!direction && !horizontal && !controlsHeld)
      inputArmed = true;
    return MenuAction::None;
  }
  int count = paused ? 5 : 6;
  int tapped=NativeTakeMenuTap(paused);
  if(tapped>=0 && tapped<count){selection=tapped;confirm=true;}

  if (direction && heldDirection != direction) {
    selection = (selection + direction + count) % count;
    fprintf(stderr, "MENU selection %d\n", selection);
  }
  heldDirection = direction;
  bool adjust = horizontal && heldHorizontal != horizontal;
  heldHorizontal = horizontal;
  if (back) {
    selection = count - 1;
    confirm = false;
    adjust = false;
  }
  int graphicsRow = paused ? 1 : 2;
  if (selection == graphicsRow && (confirm || adjust)) return ToggleArt(adjust ? horizontal : 1);
  if (!paused && selection == 1 && (confirm || adjust)) {
    int delta = adjust ? horizontal : 1;
    players = (players - 1 + delta + 4) % 4 + 1;
    fprintf(stderr, "MENU players %d\n", players);
    return MenuAction::None;
  }
  int musicRow = paused ? 2 : 3;
  if (selection == musicRow && (confirm || adjust)) return ToggleMusic(adjust ? horizontal : 1);
  if (!paused && selection == 4 && (confirm || adjust)) {
    megaWeapons = !megaWeapons;
    fprintf(stderr, "MENU mega weapons %s\n", megaWeapons ? "ON" : "OFF");
    return MenuAction::None;
  }
  if (!confirm) return MenuAction::None;
  if (paused) {
    if (selection == 0) return MenuAction::Resume;
    if (selection == 3) return MenuAction::Title;
    return MenuAction::Exit;
  }
  if(selection==0){NativePrepareGameplayController(starterPad);return MenuAction::Start;}
  return MenuAction::Exit;
}
MenuAction Frontend::ToggleMusic(int delta) {
  int next = (NativeMusicChoice() + delta + 4) % 4;
  if (XEngine->SelectMusic(next)) {
    fprintf(stderr, "MENU music %s\n", NativeMusicLabel(next));
    if (paused && !Game.m_PlayerList.empty()) {
      const auto *player = Game.m_PlayerList.front();
      fprintf(stderr, "MUSIC live switch; score=%d players=%d x=%.2f y=%.2f power=%d\n",
              player->m_Score, Game.m_NumPlayers, player->m_PlayerPos.x,
              player->m_PlayerPos.y, player->m_Weapon->m_PowerLevel);
    }
  }
  return MenuAction::None;
}
MenuAction Frontend::ToggleArt(int delta) {
  // Skip missing packs in either direction; Original always remains available.
  int next = artMode;
  for (int attempt = 0; attempt < 6; ++attempt) {
    next = (next + delta + 6) % 6;
    if (NativeSetArtMode(next)) break;
  }
  if (next == artMode) return MenuAction::None;
  artMode = next;
  backdrop = XEngine->LoadTexture("d:\\data\\backgrounds\\train_bg3.png");
  fprintf(stderr, "MENU graphics %s\n", NativeArtLabel(artMode));
  return MenuAction::ArtChanged;
}

void Frontend::Draw() {
  const float t=float(std::fmod(GetTickCount64()/1000.0,3600.0));
  XEngine->SetCameraPos(0,0);
  XEngine->SetSpriteMode(SM_ALPHA_BLEND);
  XEngine->SetMaterial(1,1,1,1);
  if(paused) Game.Refresh();
  else XEngine->DrawSprite(backdrop,320+std::sin(t*.13f)*6,240+std::cos(t*.11f)*4,
                           680.f*backdrop->width/backdrop->height,680,0);
  XEngine->SetCameraPos(0,0);
  XEngine->FillRect(-20,-20,680,520,.018f,.014f,.011f,paused?.78f:.66f);
  XEngine->FillRect(22,22,596,1,.32f,.24f,.16f,.8f);
  XEngine->FillRect(22,457,596,1,.32f,.24f,.16f,.8f);
  XEngine->DrawUiText("DEAD BABIES X",322,44,32,49,22,15,true);
  XEngine->DrawUiText("DEAD BABIES X",320,42,32,245,228,199,true);
  XEngine->FillRect(276,84,88,2,.78f,.23f,.13f,1);
  XEngine->DrawUiText(paused?"PAUSED":"XTREME EDITION",320,98,12,195,148,104,true);
  const int count=paused?5:6;
  const float panelY=paused?150.f:130.f;
  const float top=paused?190.f:170.f;
  const float rowHeight=35.f;
  NativeUiPanel(112,panelY,416,paused?236.f:264.f);
  XEngine->DrawUiText(paused?"ROUND PAUSED":"ARENA SETUP",132,panelY+13,10,175,149,116,false);
  XEngine->FillRect(462,panelY+16,4,4,paused?.85f:.38f,paused?.44f:.63f,.20f,1);
  XEngine->DrawUiText(paused?"HOLD":"READY",510,panelY+13,10,160,145,122,true);
  XEngine->FillRect(132,panelY+30,376,1,.22f,.17f,.12f,1);
  char playersText[40],graphicsText[64],musicText[64],megaText[48];
  snprintf(playersText,sizeof(playersText),"PLAYERS: < %d >",players);
  snprintf(graphicsText,sizeof(graphicsText),"GRAPHICS: < %s >",NativeArtLabel(artMode));
  snprintf(musicText,sizeof(musicText),"MUSIC: < %s >",NativeMusicLabel(NativeMusicChoice()));
  snprintf(megaText,sizeof(megaText),"MEGA WEAPONS: < %s >",megaWeapons?"ON":"OFF");
  const char *titleItems[]={"START GAME",playersText,graphicsText,musicText,megaText,"EXIT GAME"};
  const char *pauseItems[]={"RESUME",graphicsText,musicText,"TITLE SCREEN","EXIT GAME"};
  for(int i=0;i<count;++i) {
    const float y=top+i*rowHeight;const bool active=i==selection;
    XEngine->FillRect(132,y-4,376,30,active?.27f:.075f,active?.075f:.062f,active?.042f:.051f,1);
    if(active) {
      XEngine->FillRect(132,y-4,3,30,.96f,.32f,.16f,1);
      XEngine->FillRect(135,y+25,373,1,.46f,.19f,.09f,1);
    }
    char number[8];snprintf(number,sizeof(number),"%02d",i+1);
    XEngine->DrawUiText(number,150,y+4,10,active?221:100,active?154:91,active?92:77,true);
    const bool longGraphics=i==(paused?1:2)&&artMode==3;
    XEngine->DrawUiText(paused?pauseItems[i]:titleItems[i],334,y+1,longGraphics?14:16,
                       active?255:168,active?234:155,active?204:136,true);
  }
  const char *hint="TAP A ROW TO SELECT";
  if(!paused&&selection==0)hint="TAP START GAME TO PLAY";
  if(!paused&&selection==1)hint="LEFT / RIGHT: PLAYER COUNT";
  if(selection==(paused?1:2))hint="LEFT / RIGHT: GRAPHICS STYLE";
  if(selection==(paused?2:3))hint="LEFT / RIGHT: CHANGE TRACK";
  if(!paused&&selection==4)hint=megaWeapons?"MAX WEAPON LEVEL: 5":"MAX WEAPON LEVEL: 3";
  XEngine->DrawUiText(hint,320,414,10,190,159,120,true);
  XEngine->DrawUiText("TAP A ROW: SELECT     BACK: PAUSE",320,438,10,157,143,122,true);
}
