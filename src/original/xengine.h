#ifndef _H_XENGINE_H
#define _H_XENGINE_H

#include <xtl.h>
#include <dmusici.h>
#include <xfont.h>

#include <math.h>
#include <stdio.h>
#include <list>
#include <xgraphics.h>
//#include <xbfont.h>
#include <dmusici.h>
#include <dsstdfx.h>
#include <dsound.h>

#include "wmainmemory.h"
//#include "wmvplayer.h"

#define XE_MUSIC_QUANTUM		1000 / 60

#define XE_XBINPUT_DEADZONE		0.30f //may need to be a LITTLE more even

struct XBGAMEPAD : public XINPUT_GAMEPAD
{

    // The following members are inherited from XINPUT_GAMEPAD:
    //    WORD    wButtons;
    //    BYTE    bAnalogButtons[8];
    //    SHORT   sThumbLX;
    //    SHORT   sThumbLY;
    //    SHORT   sThumbRX;
    //    SHORT   sThumbRY;

    // Thumb stick values converted to range [-1,+1]
    FLOAT      fX1;
    FLOAT      fY1;
    FLOAT      fX2;
    FLOAT      fY2;
    
    // State of buttons tracked since last poll
    WORD       wLastButtons;
    BOOL       bLastAnalogButtons[8];
    WORD       wPressedButtons;
    BOOL       bPressedAnalogButtons[8];

    // Rumble properties
    XINPUT_RUMBLE   Rumble;
    XINPUT_FEEDBACK Feedback;

    // Device properties
    XINPUT_CAPABILITIES caps;
    HANDLE     hDevice;

    // Flags for whether game pad was just inserted or removed
    BOOL       bInserted;
    BOOL       bRemoved;
};

struct SPRITEVERTEX
{
    D3DXVECTOR3 Pos;
    D3DXVECTOR2 Tex;
};

enum SPRITEMODE
{
	SM_ALPHA_BLEND	= 0,
	SM_ALPHA_ADD	= 1
};

struct COLLISIONINFO
{
	D3DXVECTOR2 Pos;
	bool Collided;

	COLLISIONINFO()
	{
		Collided=false;
	}
};

enum SOUNDTYPE
{
	ST_NORMAL	= 0,
	ST_LOOPING	= 1
};

class CSound
{
protected:

	IDirectMusicSegment8*     m_pSoundSegment;

public:
	void Create(char* filename);
	void PlaySound();
	void PlaySound(IDirectMusicSegmentState **Channel);

	void Release();
	void SetSoundType(int Type);

protected:

	void LoadSound(char* filename);
};

class CXEngine
{
private:

	//d3d instance and device
	LPDIRECT3D8			m_pD3D;
	LPDIRECT3DDEVICE8	m_pD3DDevice;
	IDirect3DSurface8*	m_BackBuffer;

	D3DXMATRIX				m_matProj;
	LPDIRECT3DVERTEXBUFFER8	m_SpriteVertexBuffer;

	XFONT*					m_pMicroFont36Font;
	XFONT*					m_pMicroFont24Font;
	XFONT*					m_pMicroFont18Font;

	IDirect3DTexture8 *m_MotionBlurTexture;
	HRESULT CopyRects(D3DSurface *pSurfaceSrc, CONST RECT *pSrcRectsArray, UINT cRects, D3DSurface *pSurfaceDst, CONST POINT *pDstPointsArray);

	//dsound & music
public:

	IDirectMusicLoader8*      m_pLoader;
	IDirectMusicPerformance8* m_pPerformance;
	IDirectMusicAudioPath8*	  m_MusicAudioPath;

	int m_SoundsPerFrame;

	//awkldgjklj
	float m_Ticks,m_GameSpeed;

private:

	LPDIRECTSOUND8 m_pDSound;

	CWMAFileStream	m_MusicStream;
	HANDLE			m_MusicThread;
	bool			m_MusicPlaying;

	void DownloadEffectsImage(PCHAR pszScratchFile);

	//video

//	CWMVPlayer m_WMVPlayer;
    LPDIRECT3DTEXTURE8  m_pOverlay[2];  // Overlay textures
    LPDIRECT3DSURFACE8  m_pSurface[2];  // Overlay Surfaces
    DWORD               m_dwCurrent;    // Current surface

	//timing
public:
	LARGE_INTEGER m_Millisecs;
private:
	LARGE_INTEGER m_LastMillisecs;
	float m_Tween,m_FPS;

	//input
	XINPUT_STATE	InputStates[4];

	//subsystem functions
	void GraphicsInit();
	void GraphicsUpdate();
	
	void TimingInit();
	void TimingUpdate();

	void SoundInit();
	void SoundUpdate();

	void InputInit();
	void InputUpdate();

public:

	void XInit();
	void XUpdate();

	//input (needs functions and privatization)
	XBGAMEPAD		GamePad[4];

	//graphics

	IDirect3DTexture8* LoadTexture(char *pSrcFile);

	void EnableMotionBlur();
	void DisableMotionBlur();
	void RenderMotionBlur(float Alpha);
	
	void SetMaterial(float Red,float Green,float Blue,float Alpha);
	void SetSpriteMode(int SpriteMode);
	void DrawSprite(IDirect3DTexture8* Texture,float PosX,float PosY,float SizeX,float SizeY,float Rot);
	void DrawSpriteOC(IDirect3DTexture8* Texture,float PosX,float PosY,float SizeX,float SizeY,float Rot);
	void DrawTracer(IDirect3DTexture8* Texture,float X1,float Y1,float X2,float Y2,float Width);

	void DrawText(char *strText,int PosX,int PosY,int Red,int Green,int Blue);
	void DrawMenuText(char *strText,int PosX,int PosY,int Red,int Green,int Blue);
	void DrawScoreText(char *strText,int PosX,int PosY,int Red,int Green,int Blue);

	void SetCameraPos(float PosX,float PosY);

	COLLISIONINFO Collide(float X1,float Y1,float R1,float DX,float DY,float X2,float Y2,float R2);

	inline float GetTween(){return m_Tween;}

	//sound & music

	IDirectSoundBuffer8* LoadSound(char *pSrcFile);

	friend DWORD WINAPI MusicProc( LPVOID lpParameter );

	void PlayMusic(const CHAR* strFileName);
	void PauseMusic();
	void SetMusicVolume(int Volume);
	void StopChannel(IDirectMusicSegmentState *Channel);

	void SetI3DL2Listener(LPCDSI3DL2LISTENER pds3dl);

	void SetGameSpeed(float Speed);
	//video

	HRESULT CXEngine::PlayVideo(char* strFilename);

};

extern CXEngine *XEngine;

float Sin(float Angle);
float Cos(float Angle);
float ATan2(float DeltaX,float DeltaY);
float fAbs(float Num);
float Rand();
float Pow(float Num,float Degree);
float Sqrt(float Num);

#endif