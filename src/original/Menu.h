#ifndef _H_MENU
#define _H_MENU

#include "xengine.h"
#include <dxfile.h>
/*
enum MENUS
{
	M_MAINMENU		=0
};

enum MAINMENU
{
	MM_SINGLEPLAYER	=0,//profile then select senario
	MM_MULTIPLAYER	=1,//both select profiles, then select game type
						//, then map
	MM_PROFILES		=2,//create new ones/delete
	MM_PLAYMOVIE	=3 //play the movies!!!
};
*/

struct PROFILESTATS
{
	int HighScore;
};

struct PROFILE
{
	char* ProfileName;
	int ProfileColor;
	D3DXVECTOR2 JoystickSensitivity;
	bool InvertYAxis;
	bool RelativeAiming;
	PROFILESTATS Stats;
	int CodesList[1];
	PROFILE();
};

struct GAME_SETTINGS
{
	int GameMode;
	PROFILE *Profiles[4];
};


/*
class PScreen //Profile Screen
{
public:

};

class CMenu
{
public:

	void Init();
	void Refresh();
	void DrawProfileList();
	void RefreshProfileScreen(int NumberOfPlayers);

	int Update();
	int ToLetters();

	HANDLE m_MyHandle;
	HANDLE m_GameHandle;

	IDirectXFileSaveObject * ProfileSaveObject;

	float m_XposDist;

	int m_SelectY;
	int m_CurrentMenu;		
	int m_NumOfProfiles;
	int m_NumberofPlayers;

	bool m_DispChars[4];
	bool m_ReadyBegin[4];
	bool m_DispProf[4];
	bool m_FirstRot;

	CSound m_ChangeSound;
	CSound m_SelectSound;

	std::list<PROFILE*>		m_ProfileList;

	IDirect3DTexture8 *m_BackGroundImage[1];
	IDirect3DTexture8 *m_ProfileColor[13];

};

extern CMenu Menu;
*/

#endif
