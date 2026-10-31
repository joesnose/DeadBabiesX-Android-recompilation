#pragma once
#include <cstdint>
#include <string>
struct XBGAMEPAD;
struct LanConfig { int slot=0, players=0; uint32_t seed=0; bool mega=false; };
void LanHost(bool mega);
void LanJoin(const std::string& ip);
void LanStart();
void LanCancel();
void LanStop();
std::string LanStatus();
bool LanTakeReady(LanConfig&);
bool LanActive();
uint32_t LanSeed();
int LanPlayerSlot();
bool LanExchange(XBGAMEPAD* pads,uint64_t hash,bool& paused,void(*pump)());
uint64_t LanGameHash();
void NativeShowLanSetup(bool mega);
void NativeLanPump();

bool LanLobbyOpen();

enum class LanAction {None,PlayAgain,MainMenu};
LanAction LanTakeAction();
int LanMenuMode();
int LanMenuSelection();
