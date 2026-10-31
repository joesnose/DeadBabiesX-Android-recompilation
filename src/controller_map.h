#pragma once
#include <SDL.h>
struct XBGAMEPAD;
void ControllerMapLoad(const char*path="controller-bindings-v1.txt");
void ControllerMapOpen();
bool ControllerMapActive();
bool ControllerMapEvent(const SDL_Event&);
void ControllerMapTick();
void ControllerMapDraw();
void ControllerMapApply(SDL_GameController*,XBGAMEPAD&);
int ControllerMapSelfTest();
