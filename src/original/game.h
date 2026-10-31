#ifndef _H_GAME
#define _H_GAME

#include "xengine.h"
#include "menu.h"

#define W_MAX_ROCKET_FUEL			300.0f
#define W_ROCKET_ACCELERATION		0.25f
#define W_ROCKET_DECELERATION		0.98f
//#define XINPUT_GAMEPAD_START        0x00000010

#define W_NUM_WEAPONS				7
#define W_NUM_SPECIALS				5

#define G_BABY_FLOAT_SPEED			0.5f;
#define G_GORE_LEVEL				500;

struct ScreenShake
{
	float m_Life,m_Amplitude,m_Speed;
};

struct Smoke
{
	D3DXVECTOR2 m_Pos,m_Speed;
	float m_Life,m_Rot;
	bool m_RocketSmoke;
};

struct Bullet
{
	D3DXVECTOR2 m_Pos,m_Speed;
	D3DXVECTOR2 m_oPos,m_sPos;

	class CPlayer* m_Player;

	float m_Life;
	int m_PowerLevel;
};

struct Plasma
{
	D3DXVECTOR2 m_Pos,m_Speed;

	class CPlayer* m_Player;

	float m_Life,m_Rot;
	int m_PowerLevel;
};

struct Grenade
{
	D3DXVECTOR2 m_Pos,m_Speed;

	class CPlayer* m_Player;
	class CWeapon* m_Launcher;

	float m_Life,m_Rot,m_RotSpeed;
	int m_PowerLevel;
};

struct Slug
{
	D3DXVECTOR2 m_Pos;
	D3DXVECTOR2 m_Speed;

	bool m_isIon;
	float m_Life;
	int m_PowerLevel;
};

struct Fire
{
	D3DXVECTOR2 m_Pos,m_Speed;
	float m_Life,m_Rot;
	bool m_SmallFire;
};

struct Flame
{
	D3DXVECTOR2 m_Pos,m_Speed;
	float m_Life,m_Rot;
	float m_RotSpeed;

	class CPlayer* m_Player;
	class CWeapon* m_Launcher;

	int m_PowerLevel;
};

struct Corona
{
	D3DXVECTOR2 m_Pos;
	float m_Life, m_Rot;
};

struct Spark
{
	D3DXVECTOR2 m_Pos,m_Speed;
	float m_Life,m_Rot;
};

struct BulletSpark
{
	D3DXVECTOR2 m_Pos,m_Speed;
	float m_Life;
};

struct Balloon
{
	D3DXVECTOR2 m_Pos,m_Speed;
};

struct Score
{
	D3DXVECTOR2 m_Pos;
	int m_Value,m_Red,m_Green,m_Blue;
	float m_Life;
};

struct Gib
{
	D3DXVECTOR2 m_Pos,m_Speed;
	float m_Life,m_Rot,m_RotDelta;
	int m_GibType;
};

enum WEAPONTYPE
{
	WT_RANDOM		= 0,
	WT_RAILGUN		= 1,
	WT_MACHINEGUN	= 2,
	WT_ROCKETS		= 3,
	WT_PLASMA		= 4,
	WT_GRENADES		= 5,
	WT_FLAMETHROWER = 6
};

enum SPECIALTYPE
{
	ST_NUKE			= 0,
	ST_SHIELD		= 1,
	ST_GRAVITY		= 2,
	ST_TIME			= 3,
	ST_IONCANNON	= 4
};

enum PLAYERSTATE
{
	PS_STAND		= 0,
	PS_RUNRIGHT		= 1,
	PS_RUNLEFT		= 2,
};

enum PLAYERCONDITION
{
	PC_ALIVE		= 0,
	PC_INVINCIBLE	= 1,
	PC_DEAD			= 2
};

enum BABYSTATE
{
	BS_FLOATINGLEFT		= 0,
	BS_FLOATINGRIGHT	= 1,
	BS_BOUNCING			= 2,
	BS_GETTINGUP		= 3,
	BS_WALKLEFT			= 4,
	BS_WALKRIGHT		= 5
};

enum WEAPONBOXSTATE
{
	WBS_FLOATING		= 0,
	WBS_FALLING			= 1
};

enum FLOORMATERIAL
{
	FM_GENERIC			= 0,
	FM_METAL			= 1,
	FM_WATER			= 2,
};

class CLevel
{
public:

	int m_FloorMaterial;
};

class CSpecial
{
public:

	bool m_Activated;

	class CPlayer *m_Player;
	float m_Life;
	float m_FireDelay;

	virtual void Activate();
	virtual void Update();
};

class CShield : public CSpecial
{
public:

	CShield();

	void Activate();
	void Update();
};

class CNuke : public CSpecial
{
public:

	class CRockets* m_Launcher;

	CNuke();

	void Activate();
	void Update();
};

class CGravity : public CSpecial
{
public:

	CGravity();

	void Activate();
	void Update();
};

class CTime : public CSpecial
{
public:

	CTime();

	void Activate();
	void Update();
};

class CIonCannon : public CSpecial
{
public:

	CIonCannon();

	void Activate();
	void Update();
};

class CWeapon
{
public:
	
	class CPlayer *m_Player;

	D3DXVECTOR2 m_Target;
	D3DXVECTOR2 m_HotPoint;

	int m_PowerLevel;
	int m_WeaponType;
	float m_Charge;

	virtual void Update();
	virtual void Fire();
	virtual void FireAlternate();
	virtual void Stop();
};

class CGrenades : public CWeapon
{
public:

	CGrenades();

	float m_FireDelay;

	void Update();
	void Fire();
	void FireAlternate();
};

class CPlasma : public CWeapon
{
public:

	CPlasma();

	float m_FireDelay;

	void Update();
	void Fire();
	void FireAlternate();
};

class CFlameThrower : public CWeapon
{
public:

	CFlameThrower();

	float m_FireDelay;

	bool m_Firing;

	IDirectMusicSegmentState *m_SoundState[6];

	void Update();
	void Fire();
	void FireAlternate();
	void Stop();
};

class CRockets : public CWeapon
{
public:

	int m_NumRockets;
	float m_FireDelay;
	int m_DumbFire;

	CRockets();

	void Update();
	void Fire();
	void FireAlternate();

};

class CRocket
{
public:


	void Update();
	CRockets* m_Launcher;

	D3DXVECTOR2 m_Pos;
	D3DXVECTOR2 m_Speed;
	float m_Rot;

	float m_Fuel;
	int m_PowerLevel;

	IDirectMusicSegmentState *SoundState;

	~CRocket()
	{
		XEngine->StopChannel(SoundState);
	}
	
};


//-----------------------------------------------------------------------
class CRailgun : public CWeapon
{
public:

	CRailgun();

	void Fire();
	void FireAlternate();
	void Update();

	float m_FireDelay;
};
//-----------------------------------------------------------------------

class CMachineGun : public CWeapon
{
public:

	CMachineGun();

	void Fire();
	void FireAlternate();
	void Update();

	float m_Bullets;
	float m_Reload;
	float m_FireDelay;
	float m_FlashTime;
	int m_FlashType;
};

class CBomb
{
public:
	float m_Size;
	D3DXVECTOR2 m_Pos, m_Speed;
	float m_Rot;
};

class CBaby
{
public:

	CBaby();

	D3DXVECTOR2 m_Pos;
	D3DXVECTOR2 m_Speed;
	
	class CPlayer *m_LastHit;
	class CPlayer *m_Killer;
	class CPlayer *m_Burner;

	float m_Rot,m_DeltaRot,m_Stand,m_FrameTime;

	int m_WalkFrame;
	int m_BabyState;
	int m_NumBalloons;
	int m_ThreatLevel;
	int m_Health;
	int m_Combo;

	bool m_HasBomb;
	int m_OnFire;
	int m_Hit;

	void Damage(float VX,float VY,int NumDamage,CPlayer* Player, int damageType);
};

class CWeaponBox
{
public:
	CWeaponBox();	

	D3DXVECTOR2 m_Pos;
	D3DXVECTOR2 m_Speed;

	bool m_Special;

	int m_WeaponType;
	int m_BoxState;

	float m_Life,m_Rot;
};

class CPlayer
{
public:

	CPlayer();

	int m_Score,m_Bonus,m_ControllerNum,m_Reload;

	CWeapon *m_Weapon;

	CSpecial *m_WhiteSpecial;
	CSpecial *m_BlackSpecial;

	D3DXVECTOR2 m_AimerPos;
	D3DXVECTOR2 m_PlayerPos;

	int m_PlayerState,m_CurrentFrame;
	float m_FrameTime,m_Rot;
	int m_StepSoundTime;

	int m_PlayerCondition,m_Lives;
	float m_ConditionTime;

	struct PROFILE *m_PlayerProfile;
	int m_Direction;

	void AddScore(int NewScore,float X,float Y);

	void Init();
	void Update();
	void Shutdown();
};

class CGame
{
public:

	//globals
	CLevel *m_Level;

	//graphics
	IDirect3DTexture8 *m_BGImage[4];//should be in level class

	IDirect3DTexture8 *m_GUIBoxImage;

	IDirect3DTexture8 *m_AimerImage[4];
	IDirect3DTexture8 *m_PlayerImage[4][8];
	IDirect3DTexture8 *m_PlayerLivesImage[4];

	IDirect3DTexture8 *m_WeaponBoxImage[W_NUM_WEAPONS];
	IDirect3DTexture8 *m_SpecialBoxImage[W_NUM_SPECIALS];
	IDirect3DTexture8 *m_WeaponParachuteImage;

	IDirect3DTexture8 *m_BabyFloatingImage[3];
	IDirect3DTexture8 *m_BabyGetUpImage[4];
	IDirect3DTexture8 *m_BabyWalkImage[5];
	IDirect3DTexture8 *m_BabyDeadImage;

	IDirect3DTexture8 *m_GibImage[6];

	IDirect3DTexture8 *m_BalloonImage;
	
	IDirect3DTexture8 *m_RocketImage;
	IDirect3DTexture8 *m_GrenadeImage;
	IDirect3DTexture8 *m_SmokeImage;
	IDirect3DTexture8 *m_RocketSmokeImage;
	IDirect3DTexture8 *m_FireImage;
	IDirect3DTexture8 *m_BulletTracerImage;

	IDirect3DTexture8 *m_SlugTracerImage;
	IDirect3DTexture8 *m_CoronaImage;

	IDirect3DTexture8 *m_SparkImage;
	IDirect3DTexture8 *m_BombImage;
	IDirect3DTexture8 *m_PlasmaImage;

	IDirect3DTexture8 *m_PowerBarImage;
	IDirect3DTexture8 *m_PowerBarBoxImage;
	IDirect3DTexture8 *m_GameOverImage;

	IDirect3DTexture8 *m_MuzzleFlashImage[3];

	//sounds

	//****************************

	//insert flamethrower sound here
	CSound m_FlameThrowerRoastSound;
	CSound m_FlameThrowerFireSound;
	CSound m_FlameThrowerHitSound;

	//****************************

	CSound m_ShotgunSound;
	CSound m_MachinegunSound[4];
	CSound m_BulletHitSound[3];

	CSound m_PlasmaLaunchSound;
	CSound m_PlasmaHitSound;

	CSound m_RocketLaunchSound;
	CSound m_RocketExplodeSound;
	CSound m_RocketFlySound;

	CSound m_GrenadeLaunchSound;
	CSound m_GrenadeBounceSound[2];

	CSound m_RailgunSound;

	CSound m_StepSound[8];
	CSound m_FloorSound[4];

	CSound m_WeaponUpgradeSound;
	CSound m_WeaponPickupSound;

	CSound m_GibSound[4];

	CSound m_AmbientSound; //should be in level class

	//lists
	std::list<CPlayer*>		m_PlayerList;
	std::list<CBaby*>		m_BabyList;
	std::list<CRocket*>		m_RocketList;
	std::list<Smoke*>		m_SmokeList;
	std::list<Fire*>		m_FireList;
	std::list<ScreenShake*>	m_ScreenShakeList;
	std::list<Bullet*>		m_BulletList;
	std::list<Slug*>		m_SlugList;
	std::list<Spark*>		m_SparkList;
	std::list<Balloon*>		m_BalloonList;
	std::list<CWeaponBox*>	m_BoxList;
	std::list<BulletSpark*>	m_BulletSparkList;
	std::list<Score*>		m_ScoreList;
	std::list<Gib*>			m_GibList;
	std::list<CBomb*>		m_BombList;
	std::list<Plasma*>		m_PlasmaList;
	std::list<Grenade*>		m_GrenadeList;
	std::list<CSpecial*>	m_SpecialList;
	std::list<Flame*>		m_FlameList;
	std::list<Corona*>		m_CoronaList;

	//vars
	struct GAME_SETTINGS* m_GameSettings;

	float	m_ScrollSpeed;
	float	m_Scroll;

	float	m_BabyRate;
	float	m_BabyDelay;
	float	m_BoxDelay;
	float	m_Gravity;
    float	m_GameOverDelay;

	int m_BabiesKilled;
	int m_NumBabies;
	int m_NumPlayers;
	int m_BestScores[4];
	int m_Continues[4];
	
	bool m_Paused;

	//functions
	void Init();
	int Update();
	void Refresh();

	CGame()
	{
		m_GameSettings=new GAME_SETTINGS;
	}

};

extern CGame Game;

#endif