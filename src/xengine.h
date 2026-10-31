#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <memory>
#include <vector>
#include "platform.h"
#include "audio_mixer.h"
using std::max;
using std::min;
bool LanActive();
float Sqrt(float);
struct D3DXVECTOR2 {
  float x = 0, y = 0;
  D3DXVECTOR2() = default;
  D3DXVECTOR2(float a, float b) : x(a), y(b) {}
};
inline void D3DXVec2Normalize(D3DXVECTOR2 *o, const D3DXVECTOR2 *v) {
  float n = LanActive()?Sqrt(v->x*v->x+v->y*v->y):std::hypot(v->x, v->y);
  *o = n > 0 ? D3DXVECTOR2(v->x / n, v->y / n) : D3DXVECTOR2();
}
struct IDirect3DTexture8 {
  unsigned id = 0;
  unsigned width = 0, height = 0;
  float u0 = 0, v0 = 0, u1 = 1, v1 = 1;
  bool empty = false;
};
struct IDirectMusicSegmentState {
  uint64_t id = 0;
};
struct DSI3DL2LISTENER {
  int unused;
};
#define DSI3DL2_ENVIRONMENT_PRESET_NOREVERB 0
struct PadRumble {
  WORD wLeftMotorSpeed = 0, wRightMotorSpeed = 0;
};
struct PadFeedback {
  PadRumble Rumble;
};
struct XBGAMEPAD {
  WORD wButtons = 0, wPressedButtons = 0, wLastButtons = 0;
  BYTE bAnalogButtons[8]{}, bPressedAnalogButtons[8]{};
  bool bLastAnalogButtons[8]{};
  float fX1 = 0, fY1 = 0, fX2 = 0, fY2 = 0;
  HANDLE hDevice = nullptr;
  PadFeedback Feedback;
};
void NativeRumble(HANDLE, PadFeedback *);
#define XInputSetState NativeRumble
struct COLLISIONINFO {
  D3DXVECTOR2 Pos;
  bool Collided = false;
};
enum { SM_ALPHA_BLEND, SM_ALPHA_ADD };
enum { ST_NORMAL, ST_LOOPING };
class CSound {
  friend class CXEngine;
  std::shared_ptr<SoundData> sound;
  bool loop = false;

public:
  void Create(const char *);
  void PlaySound();
  void PlaySound(IDirectMusicSegmentState **);
  void SetSoundType(int t) { loop = t == ST_LOOPING; }
  void Release() { sound.reset(); }
};
class CXEngine {
public:
  XBGAMEPAD GamePad[4];
  float m_Ticks = 0, m_GameSpeed = 1;
  LARGE_INTEGER m_Millisecs{};
  int m_SoundsPerFrame = 16;
  void StopAllSounds(bool releaseTokens = true);
  void RequestExit();
  bool TakeMenuBack();
  void ClearInputEdges();
  void SetAudioPaused(bool);
  void FillRect(float, float, float, float, float, float, float, float);
  void DrawUiText(const char *, float, float, float, int, int, int,
                  bool centered = false);
  void DrawHudScore(const char *, float, int, int, int);
  void XInit();
  void XUpdate();
  bool Running() const;
  void Shutdown();
  float GetTween() const { return m_GameSpeed; }
  IDirect3DTexture8 *LoadTexture(const char *);
  void SetMaterial(float, float, float, float);
  void SetSpriteMode(int);
  void SetCameraPos(float, float);
  void DrawSprite(IDirect3DTexture8 *, float, float, float, float, float);
  void DrawSpriteOC(IDirect3DTexture8 *, float, float, float, float, float);
  void DrawTracer(IDirect3DTexture8 *, float, float, float, float, float);
  void DrawText(const char *, int, int, int, int, int);
  void DrawScoreText(const char *, int, int, int, int, int);
  void DrawMenuText(const char *, int, int, int, int, int);
  void EnableMotionBlur();
  void DisableMotionBlur();
  void RenderMotionBlur(float);
  COLLISIONINFO Collide(float, float, float, float, float, float, float, float);
  void PlayMusic(const char *);
  bool SelectMusic(int choice);
  void BeginResultsAudio();
  void PlayResultCue(bool winner);
  void PauseMusic();
  void SetMusicVolume(int);
  void StopChannel(IDirectMusicSegmentState *);
  void SetI3DL2Listener(const DSI3DL2LISTENER *) {};
  void SetGameSpeed(float);

private:
  float cameraX = 0, cameraY = 0;
};
extern CXEngine *XEngine;
float Sin(float);
float Cos(float);
float ATan2(float, float);
float fAbs(float);
float Rand();
float Pow(float, float);
float Sqrt(float);

// Prefer the faithful HD skin when installed; original art can be forced.
const char *NativeArtSkin();

bool NativeSetHDArt(bool enabled);
bool NativeHasHDArt();

// 0 = Original, 1 = HD, 2 = Super HD, 3 = Super Super HD, 4 = Cel Shaded, 5 = Cartoon. Missing packs are rejected.
bool NativeHasArtMode(int mode);
bool NativeSetArtMode(int mode);
const char *NativeArtLabel(int mode);

int NativeMusicChoice();
const char *NativeMusicLabel(int choice);

void NativeSetTouchScene(bool,bool);
int NativeTakeMenuTap(bool);

void NativePrepareGameplayController(int slot);

#ifndef __ANDROID__
bool NativeIsHeadlessInputTest();
#endif

#ifndef __ANDROID__
void NativeSetHeadlessInputTest(bool);
#endif

XBGAMEPAD NativeLanReadLocalInput();
#ifndef __ANDROID__
bool NativeLanLatchSelfTest();
#endif
