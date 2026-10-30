// FLAMETHROWER SOUND
// FLAMETHROWER ALT FIRE (SHOULD HAVE SOME RANGE)
// STARDRIVER ALT-FIRE
// GRENADE LAUNCHER ALT-FIRE
// ROCKET ALTI FIRE SHOULD BE AUTOGUIDED, NOT HOMING TOWARDS CURSOR
// WORK WITH THE COOL ROTATION EFFECTS
// GET THAT CLEVEL CLASS UP AND RUNNING WITH A LIST OF TIMED EVENTS AND OTHER
// SHIT LIKE THAT BIG RE-ORAGANIZATION!!

#include "game.h"
#include "lan.h"
#include "sync_rng.h"
#define rand NativeGameRandom
#define srand NativeGameSeed
#include "results_presentation.h"
#include "ui_style.h"
#include <cstring>
#include <string>
#include <climits>

CGame Game;
static int startingPlayers = 1;
static bool megaWeapons = false;
void SetMegaWeapons(bool enabled) { megaWeapons = enabled; }
int WeaponPowerLimit() { return megaWeapons ? 5 : 3; }
int CWeapon::Upgrade() {
  m_PowerLevel = std::min(m_PowerLevel + 1, WeaponPowerLimit());
  return m_PowerLevel;
}
int GrenadeFragments(int level, bool manual) {
  level = std::clamp(level, 1, 5);
  return manual ? 20 * level : 8 + (level - 1) * 12;
}
float BulletKnockbackDivisor(int level) {
  const float divisor[] = {20.f, 15.f, 10.f, 7.5f, 6.f};
  return divisor[std::clamp(level, 1, 5) - 1];
}
void SetStartingPlayers(int count) {
  startingPlayers = std::clamp(count, 1, 4);
}
static std::vector<std::unique_ptr<CWeapon>> ownedWeapons;
static std::vector<std::unique_ptr<CPlayer>> ownedPlayers;
template <class T> T *OwnWeapon() {
  auto p = std::make_unique<T>();
  auto *out = p.get();
  ownedWeapons.push_back(std::move(p));
  return out;
}
static CPlayer *OwnPlayer() {
  auto p = std::make_unique<CPlayer>();
  auto *out = p.get();
  ownedPlayers.push_back(std::move(p));
  return out;
}
void ReleaseNativeRound() {
  for (auto *p : Game.m_BabyList)
    delete p;
  Game.m_BabyList.clear();
  for (auto *p : Game.m_RocketList)
    delete p;
  Game.m_RocketList.clear();
  for (auto *p : Game.m_SmokeList)
    delete p;
  Game.m_SmokeList.clear();
  for (auto *p : Game.m_FireList)
    delete p;
  Game.m_FireList.clear();
  for (auto *p : Game.m_ScreenShakeList)
    delete p;
  Game.m_ScreenShakeList.clear();
  for (auto *p : Game.m_BulletList)
    delete p;
  Game.m_BulletList.clear();
  for (auto *p : Game.m_SlugList)
    delete p;
  Game.m_SlugList.clear();
  for (auto *p : Game.m_SparkList)
    delete p;
  Game.m_SparkList.clear();
  for (auto *p : Game.m_BalloonList)
    delete p;
  Game.m_BalloonList.clear();
  for (auto *p : Game.m_BoxList)
    delete p;
  Game.m_BoxList.clear();
  for (auto *p : Game.m_BulletSparkList)
    delete p;
  Game.m_BulletSparkList.clear();
  for (auto *p : Game.m_ScoreList)
    delete p;
  Game.m_ScoreList.clear();
  for (auto *p : Game.m_GibList)
    delete p;
  Game.m_GibList.clear();
  for (auto *p : Game.m_BombList)
    delete p;
  Game.m_BombList.clear();
  for (auto *p : Game.m_PlasmaList)
    delete p;
  Game.m_PlasmaList.clear();
  for (auto *p : Game.m_GrenadeList)
    delete p;
  Game.m_GrenadeList.clear();
  for (auto *p : Game.m_SpecialList)
    delete p;
  Game.m_SpecialList.clear();
  for (auto *p : Game.m_FlameList)
    delete p;
  Game.m_FlameList.clear();
  for (auto *p : Game.m_CoronaList)
    delete p;
  Game.m_CoronaList.clear();

  for (auto &p : ownedPlayers) {
    delete p->m_PlayerProfile;
  }
  Game.m_PlayerList.clear();
  ownedWeapons.clear();
  ownedPlayers.clear();
  delete Game.m_Level;
  delete Game.m_GameSettings;
  Game = CGame{};
  XEngine->StopAllSounds();
  XEngine->SetGameSpeed(1);
}
void ResetNativeRound(){
  ReleaseNativeRound();
  Game.Init();
}

void EnemyExplode(float PosX, float PosY, CPlayer *Player) {
  Fire *MyFire;

  for (int i = 0; i < 8.0f; i++) {
    MyFire = new Fire{};

    MyFire->m_SmallFire = false;

    MyFire->m_Pos.x = PosX + Rand() * 10.0f - 5.0f;
    MyFire->m_Pos.y = PosY + Rand() * 10.0f - 5.0f;

    MyFire->m_Speed.x = Rand() * 3.0f - 1.5f;
    MyFire->m_Speed.y = Rand() * 3.0f - 1.5f;

    MyFire->m_Life = 50.0f;
    MyFire->m_Rot = Rand() * 360.0f;

    Game.m_FireList.push_back(MyFire);
  }

  ScreenShake *MyScreenShake;

  MyScreenShake = new ScreenShake{};

  MyScreenShake->m_Life = 20.0f;
  MyScreenShake->m_Speed = 50.0f;
  MyScreenShake->m_Amplitude = 4.0f;

  Game.m_ScreenShakeList.push_back(MyScreenShake);

  for (std::list<CBaby *>::iterator iBaby = Game.m_BabyList.begin(),
                                    iBaby_next = iBaby;
       iBaby != Game.m_BabyList.end(); iBaby = iBaby_next) {
    iBaby_next = iBaby;
    ++iBaby_next;
    CBaby *Baby = (*iBaby);
    D3DXVECTOR2 Delta;

    Delta.x = Baby->m_Pos.x - PosX;
    Delta.y = Baby->m_Pos.y - PosY;

    float Dist = Sqrt(Delta.x * Delta.x + Delta.y * Delta.y);

    D3DXVec2Normalize(&Delta, &Delta);

    if (Dist < 40.0f) {
      if (Baby->m_BabyState == BS_WALKLEFT ||
          Baby->m_BabyState == BS_WALKRIGHT) {
        Baby->m_LastHit = NULL;
        Baby->m_Health = -1;

        if (Player != NULL) {
          Player->AddScore(250, Baby->m_Pos.x, Baby->m_Pos.y);
        }
      }
    }
  }

  for (std::list<CPlayer *>::iterator iPlayer = Game.m_PlayerList.begin(),
                                      iPlayer_next = iPlayer;
       iPlayer != Game.m_PlayerList.end(); iPlayer = iPlayer_next) {
    iPlayer_next = iPlayer;
    ++iPlayer_next;
    CPlayer *Player = (*iPlayer);

    if (Player->m_PlayerCondition == PC_ALIVE) {
      D3DXVECTOR2 Delta;

      Delta.x = Player->m_PlayerPos.x - PosX;
      Delta.y = Player->m_PlayerPos.y - PosY;

      float Dist = Sqrt(Delta.x * Delta.x + Delta.y * Delta.y);

      D3DXVec2Normalize(&Delta, &Delta);

      if (Dist < 40.0f) {
        Player->m_PlayerCondition = PC_DEAD;
        Player->m_ConditionTime = 180.0f;

        Gib *MyGib;

        for (int i = 0; i < 25; i++) {
          MyGib = new Gib{};

          MyGib->m_Speed.x = Rand() * 16.0f - 8.0f;
          MyGib->m_Speed.y = Rand() * 16.0f - 8.0f;

          MyGib->m_Pos.x = Player->m_PlayerPos.x;
          MyGib->m_Pos.y = Player->m_PlayerPos.y;

          MyGib->m_Life = 320.0f;
          MyGib->m_Rot = Rand() * 360.0f;

          MyGib->m_RotDelta = Rand() * 32.0f - 16.0f;

          MyGib->m_GibType = rand() % 6;

          Game.m_GibList.push_back(MyGib);
        }

        Game.m_GibSound[3].PlaySound();
      }
    }
  }

  Game.m_RocketExplodeSound.PlaySound();
}

void Explode(float PosX, float PosY, CPlayer *Player) {
  Fire *MyFire;

  for (int i = 0; i < 8.0f; i++) {
    MyFire = new Fire{};

    MyFire->m_SmallFire = false;

    MyFire->m_Pos.x = PosX + Rand() * 10.0f - 5.0f;
    MyFire->m_Pos.y = PosY + Rand() * 10.0f - 5.0f;

    MyFire->m_Speed.x = Rand() * 3.0f - 1.5f;
    MyFire->m_Speed.y = Rand() * 3.0f - 1.5f;

    MyFire->m_Life = 50.0f;
    MyFire->m_Rot = Rand() * 360.0f;

    Game.m_FireList.push_back(MyFire);
  }

  ScreenShake *MyScreenShake;

  MyScreenShake = new ScreenShake{};

  MyScreenShake->m_Life = 20.0f;
  MyScreenShake->m_Speed = 50.0f;
  MyScreenShake->m_Amplitude = 4.0f;

  Game.m_ScreenShakeList.push_back(MyScreenShake);

  for (std::list<CBaby *>::iterator iBaby = Game.m_BabyList.begin(),
                                    iBaby_next = iBaby;
       iBaby != Game.m_BabyList.end(); iBaby = iBaby_next) {
    iBaby_next = iBaby;
    ++iBaby_next;
    CBaby *Baby = (*iBaby);

    D3DXVECTOR2 Delta;

    Delta.x = Baby->m_Pos.x - PosX;
    Delta.y = Baby->m_Pos.y - PosY;

    float Dist = Sqrt(Delta.x * Delta.x + Delta.y * Delta.y);

    D3DXVec2Normalize(&Delta, &Delta);

    if (Dist < 80.0f) {
      Baby->Damage(Delta.x * (128.0f - Dist) / 8.0f,
                   Delta.y * (128.0f - Dist) / 8.0f, 8, Player, 0);

      if (Baby->m_BabyState == BS_BOUNCING) {
        Baby->m_OnFire = 15;
        Baby->m_Burner = Player;
      }
    }
  }

  for (std::list<CWeaponBox *>::iterator iBox = Game.m_BoxList.begin(),
                                         iBox_next = iBox;
       iBox != Game.m_BoxList.end(); iBox = iBox_next) {
    iBox_next = iBox;
    ++iBox_next;
    CWeaponBox *WeaponBox = (*iBox);

    D3DXVECTOR2 Delta;

    Delta.x = WeaponBox->m_Pos.x - PosX;
    Delta.y = WeaponBox->m_Pos.y - PosY;

    float Dist = Sqrt(Delta.x * Delta.x + Delta.y * Delta.y);

    D3DXVec2Normalize(&Delta, &Delta);

    if (Dist < 80.0f) {
      WeaponBox->m_BoxState = WBS_FALLING;
    }
  }

  Game.m_RocketExplodeSound.PlaySound();
}

// specialcode

void CSpecial::Activate() {}

void CSpecial::Update() {}

CNuke::CNuke() {
  m_Life = 700.0f;
  m_Activated = false;
}

void CNuke::Activate() {
  m_Launcher = OwnWeapon<CRockets>();
  m_Launcher->m_Player = m_Player;
  m_Launcher->m_DumbFire = 2;
}

void CNuke::Update() {
  m_FireDelay -= 1.0f * XEngine->GetTween();

  if (m_FireDelay <= 0.0f && m_Life > 200.0f) {
    m_FireDelay = 5.0f;

    CRocket *Rocket;

    Rocket = new CRocket{};

    Rocket->m_Launcher = m_Launcher;

    Rocket->m_Pos.x = 80.0f + Rand() * 600.0f;
    Rocket->m_Pos.y = -100.0f - Rand() * 100.0f;

    Rocket->m_Rot = -15.0f;

    Rocket->m_Speed.x = Sin(Rocket->m_Rot) * 5.0f;
    Rocket->m_Speed.y = Cos(Rocket->m_Rot) * 5.0f;

    Rocket->m_PowerLevel = 3;

    Rocket->m_Fuel = W_MAX_ROCKET_FUEL;

    // if (i%10==0)
    //{
    Game.m_RocketFlySound.PlaySound(&Rocket->SoundState);
    //}
    // else
    //{
    //	Rocket->SoundState=NULL;
    //}

    Game.m_RocketList.push_back(Rocket);
  }
}

CShield::CShield() {
  m_Activated = false;
  m_Life = -1.0f;
}

void CShield::Activate() {
  m_Player->m_PlayerCondition = PC_INVINCIBLE;
  m_Player->m_ConditionTime = 1000.0f;
}

void CShield::Update() {}

CGravity::CGravity() {
  m_Activated = false;
  m_Life = 800.0f;
}

void CGravity::Activate() {}

void CGravity::Update() {
  if (m_Life > 10.0f) {
    Game.m_Gravity = 0.10f;
  } else {
    Game.m_Gravity = 1.0f;
  }
}

CTime::CTime() {
  m_Activated = false;
  m_Life = 800.0f;
}

void CTime::Activate() {}

void CTime::Update() {
  float Speed;

  if (m_Life > 50.0f) {
    Speed = Cos(m_Life / 750.0f * 360.0f) / 2.0f + 0.7f;

    if (Speed > 1.0f) {
      Speed = 1.0f;
    }
  } else {
    Speed = 1.0f;
  }

  XEngine->SetGameSpeed(Speed);
}

CIonCannon::CIonCannon() {
  m_Activated = false;
  m_Life = 320.0f;
  m_FireDelay = 0.0f;
}

void CIonCannon::Activate() {}

void CIonCannon::Update() {
  m_FireDelay -= 1.0f * XEngine->GetTween();

  Slug *MySlug;

  if (m_FireDelay <= 0.0f) {
    MySlug = new Slug{};

    MySlug->m_Life = 60.0f;
    MySlug->m_isIon = true;
    MySlug->m_PowerLevel = 3;

    MySlug->m_Pos.x = 320.0f;
    MySlug->m_Pos.y = -640.0f;
    MySlug->m_Speed.x = 640.0f - (320.0f - m_Life) * 2.0f - MySlug->m_Pos.x;
    MySlug->m_Speed.y = 460.0f - MySlug->m_Pos.y;

    m_FireDelay = 0.0f;

    Game.m_SlugList.push_back(MySlug);

    for (std::list<CBaby *>::iterator iBaby = Game.m_BabyList.begin(),
                                      iBaby_next = iBaby;
         iBaby != Game.m_BabyList.end(); iBaby = iBaby_next) {
      iBaby_next = iBaby;
      ++iBaby_next;
      CBaby *Baby = (*iBaby);
      COLLISIONINFO MyCollisionInfo = XEngine->Collide(
          MySlug->m_Pos.x, MySlug->m_Pos.y, 32.0f, MySlug->m_Speed.x,
          MySlug->m_Speed.y, Baby->m_Pos.x, Baby->m_Pos.y, 16.0f);

      if (MyCollisionInfo.Collided) {
        Baby->Damage(MySlug->m_Speed.x / 150.0f, MySlug->m_Speed.y / 150.0f, 8,
                     m_Player, 0);

        Spark *MySpark;

        for (float Theta = 0.0f; Theta < 360.0f; Theta += 30.0f) {
          MySpark = new Spark{};

          MySpark->m_Pos.x = Baby->m_Pos.x;
          MySpark->m_Pos.y = Baby->m_Pos.y;

          MySpark->m_Speed.x = Sin(Theta) * 8.0f;
          MySpark->m_Speed.y = Cos(Theta) * 8.0f;

          MySpark->m_Life = 30.0f;
          MySpark->m_Rot = Rand() * 360.0f;

          Game.m_SparkList.push_back(MySpark);
        }
      }
    }

    for (std::list<CWeaponBox *>::iterator iBox = Game.m_BoxList.begin(),
                                           iBox_next = iBox;
         iBox != Game.m_BoxList.end(); iBox = iBox_next) {
      iBox_next = iBox;
      ++iBox_next;
      CWeaponBox *WeaponBox = (*iBox);

      if (WeaponBox->m_BoxState == WBS_FLOATING) {
        COLLISIONINFO MyCollisionInfo = XEngine->Collide(
            MySlug->m_Pos.x, MySlug->m_Pos.y, 32.0f, MySlug->m_Speed.x,
            MySlug->m_Speed.y, WeaponBox->m_Pos.x, WeaponBox->m_Pos.y, 24.0f);

        if (MyCollisionInfo.Collided) {
          WeaponBox->m_BoxState = WBS_FALLING;

          Spark *MySpark;

          for (float Theta = 0.0f; Theta < 360.0f; Theta += 30.0f) {
            MySpark = new Spark{};

            MySpark->m_Pos.x = WeaponBox->m_Pos.x;
            MySpark->m_Pos.y = WeaponBox->m_Pos.y;

            MySpark->m_Speed.x = Sin(Theta) * 8.0f;
            MySpark->m_Speed.y = Cos(Theta) * 8.0f;

            MySpark->m_Life = 30.0f;
            MySpark->m_Rot = Rand() * 360.0f;

            Game.m_SparkList.push_back(MySpark);
          }
        }
      }
    }

    Game.m_PlasmaLaunchSound.PlaySound();
  }
}

//----------------------------------------------------------------------------------------
// Weapon Code
//----------------------------------------------------------------------------------------

void CWeapon::Update() {}

void CWeapon::Fire() {}

void CWeapon::FireAlternate() {}

void CWeapon::Stop() {}

//--------------------------------------------------------------------

CGrenades::CGrenades() {
  m_FireDelay = 0.0f;

  m_PowerLevel = 1;
  m_WeaponType = WT_GRENADES;
}

void CGrenades::Fire() {
  if (m_FireDelay <= 0.0f) {

    m_FireDelay = 20.0f;

    Grenade *MyGrenade;

    MyGrenade = new Grenade{};

    MyGrenade->m_Launcher = this;
    MyGrenade->m_Player = m_Player;

    MyGrenade->m_Pos.x = m_HotPoint.x;
    MyGrenade->m_Pos.y = m_HotPoint.y;

    float DX, DY;

    DX = m_Player->m_AimerPos.x - m_HotPoint.x;
    DY = m_Player->m_AimerPos.y - m_HotPoint.y;

    float D = Sqrt(Sqrt(DX * DX + DY * DY) / 5.0f) * 1.5f;
    float Theta = ATan2(DX, DY);

    MyGrenade->m_Speed.x = Sin(Theta) * D;
    MyGrenade->m_Speed.y = Cos(Theta) * D;

    MyGrenade->m_PowerLevel = m_PowerLevel;

    MyGrenade->m_Rot = Rand() * 360.0f;
    MyGrenade->m_RotSpeed = Rand() * 32.0f - 16.0f;

    MyGrenade->m_Life = 60.0f;

    Game.m_GrenadeList.push_back(MyGrenade);

    Game.m_GrenadeLaunchSound.PlaySound();
  }
}

void CGrenades::Update() {
  if (m_FireDelay > 0.0f)
    m_FireDelay -= 1.0f * XEngine->GetTween();
}

void CGrenades::FireAlternate() {
  for (std::list<Grenade *>::iterator iGrenade = Game.m_GrenadeList.begin(),
                                      iGrenade_next = iGrenade;
       iGrenade != Game.m_GrenadeList.end(); iGrenade = iGrenade_next) {
    iGrenade_next = iGrenade;
    ++iGrenade_next;
    Grenade *MyGrenade = (*iGrenade);

    if (MyGrenade->m_Launcher == this) {
      Explode(MyGrenade->m_Pos.x, MyGrenade->m_Pos.y, m_Player);

      int Shrapnel = GrenadeFragments(MyGrenade->m_PowerLevel, true);

      for (int NumberofBullets = 0; NumberofBullets < Shrapnel;
           NumberofBullets++) {
        Bullet *MyBullet = new Bullet{};

        MyBullet->m_Player = MyGrenade->m_Player;

        MyBullet->m_Life = 30.0f;

        MyBullet->m_PowerLevel = 2;

        MyBullet->m_Pos.x = MyGrenade->m_Pos.x;
        MyBullet->m_Pos.y = MyGrenade->m_Pos.y;
        MyBullet->m_sPos.x = MyGrenade->m_Pos.x;
        MyBullet->m_sPos.y = MyGrenade->m_Pos.y;

        float Theta = Rand() * 360.0f;

        MyBullet->m_Speed.x = Sin(Theta) * 32.0f + Rand() * 10.0f;
        MyBullet->m_Speed.y = Cos(Theta) * 32.0f + Rand() * 10.0f;

        Game.m_BulletList.push_back(MyBullet);
      }

      std::list<Grenade *>::iterator iElement = iGrenade;

      Game.m_GrenadeList.erase(iElement);
      delete MyGrenade;
    }
  }
}

CPlasma::CPlasma() {
  m_FireDelay = 0.0f;
  m_PowerLevel = 1;
  m_Charge = 0.0f;

  m_WeaponType = WT_PLASMA;
}

void CPlasma::Update() {
  if (m_FireDelay > 0.0f) {
    m_FireDelay -= 1.0f * XEngine->GetTween();
  }
}

void CPlasma::Fire() {
  if (m_Charge < 30.0f) {
    Plasma *MyPlasma;

    if (m_FireDelay <= 0.0f) {
      for (int i = 0; i < m_PowerLevel; i++) {
        MyPlasma = new Plasma{};

        MyPlasma->m_Player = m_Player;

        MyPlasma->m_Life = 60.0f;

        MyPlasma->m_PowerLevel = 1;

        MyPlasma->m_Pos.x = m_HotPoint.x;
        MyPlasma->m_Pos.y = m_HotPoint.y;
        MyPlasma->m_Speed.x = m_Player->m_AimerPos.x - m_HotPoint.x;
        MyPlasma->m_Speed.y = m_Player->m_AimerPos.y - m_HotPoint.y;

        float Theta = ATan2(MyPlasma->m_Speed.x, MyPlasma->m_Speed.y);

        // Keep the original two-shot spread; extend the three-shot fan symmetrically.
        float offset = m_PowerLevel == 2 ? (i * 2 - 1) * 15.f
                                        : (i - (m_PowerLevel - 1) * .5f) * 15.f;
        MyPlasma->m_Speed.x = Sin(Theta + offset) * 16.f;
        MyPlasma->m_Speed.y = Cos(Theta + offset) * 16.f;

        Game.m_PlasmaList.push_back(MyPlasma);
      }

      // m_FireDelay=10.0f;
      m_FireDelay = 2.0f;

      Game.m_PlasmaLaunchSound.PlaySound();
    }
  } else {
    Plasma *MyPlasma;

    int ArcLength = m_PowerLevel;

    if (ArcLength > 2)
      ArcLength = 2;

    for (int i = -ArcLength * 60; i < ArcLength * 60; i++) {
      MyPlasma = new Plasma{};

      MyPlasma->m_Player = m_Player;

      MyPlasma->m_Life = 60.0f;

      MyPlasma->m_PowerLevel = 3;

      MyPlasma->m_Pos.x = m_HotPoint.x;
      MyPlasma->m_Pos.y = m_HotPoint.y;
      MyPlasma->m_Speed.x = m_Player->m_AimerPos.x - m_HotPoint.x;
      MyPlasma->m_Speed.y = m_Player->m_AimerPos.y - m_HotPoint.y;

      float Theta = ATan2(MyPlasma->m_Speed.x, MyPlasma->m_Speed.y);

      MyPlasma->m_Speed.x = Sin(Theta + i * 1.5f) * 16.0f;
      MyPlasma->m_Speed.y = Cos(Theta + i * 1.5f) * 16.0f;

      Game.m_PlasmaList.push_back(MyPlasma);
    }

    m_FireDelay = 30.0f;
    // m_FireDelay=2.0f;

    Game.m_PlasmaLaunchSound.PlaySound();
  }

  m_Charge = 0.0f;
}

void CPlasma::FireAlternate() {
  m_Charge += 0.4f * XEngine->GetTween();

  if (m_Charge < 30.0f) {
    for (int i = 0; i < 4; i++) {
      float DX = Sin(m_Charge * 12.0f + i * 90) * 40.0f,
            DY = Cos(m_Charge * 12.0f + i * 90) * 40.0f;

      Spark *MySpark = new Spark{};

      MySpark->m_Pos.x = m_Player->m_PlayerPos.x + DX;
      MySpark->m_Pos.y = m_Player->m_PlayerPos.y + DY;

      MySpark->m_Speed.x = -DX / 30.0f;
      MySpark->m_Speed.y = -DY / 30.0f;

      MySpark->m_Life = 30.0f;
      MySpark->m_Rot = Rand() * 360.0f;

      Game.m_SparkList.push_back(MySpark);
    }
  } else {
    m_Charge = 30.0f;
  }
}

//---------------------------------------------------------------------------------
CRockets::CRockets() {
  m_NumRockets = 0;
  m_PowerLevel = 1;
  m_WeaponType = WT_ROCKETS;
  m_DumbFire = true;
}

void CRockets::Fire() {
  if (m_FireDelay <= 0.0f && m_NumRockets <= 0) {
    m_DumbFire = true;
    m_FireDelay = 15.0f;

    for (int i = 0; i < m_PowerLevel; i++) {
      CRocket *Rocket;

      Rocket = new CRocket{};

      Rocket->m_Rot = m_Player->m_Rot;

      Rocket->m_Launcher = this;

      Rocket->m_Pos.x = m_HotPoint.x;
      Rocket->m_Pos.y = m_HotPoint.y;

      Rocket->m_Speed.x = (i - float(m_PowerLevel / 2.0f) + 0.5f) * 3.0f;
      Rocket->m_Speed.y = 0.0f;

      Rocket->m_PowerLevel = m_PowerLevel;

      // Rocket->m_Rot=180.0f;

      Rocket->m_Fuel = W_MAX_ROCKET_FUEL;

      // m_NumRockets++;

      Game.m_RocketFlySound.PlaySound(&Rocket->SoundState);

      // Rocket->SoundState->SetVolume(-512,0);

      Game.m_RocketList.push_back(Rocket);
    }

    Game.m_RocketLaunchSound.PlaySound();
  }
}

void CRockets::Update() {
  if (m_FireDelay >= 0.0f)
    m_FireDelay -= 1.0f * XEngine->GetTween();
}

void CRockets::FireAlternate() {
  if (m_NumRockets <= 0) {
    for (std::list<CRocket *>::iterator iRocket = Game.m_RocketList.begin(),
                                        iRocket_next = iRocket;
         iRocket != Game.m_RocketList.end(); iRocket = iRocket_next) {
      iRocket_next = iRocket;
      ++iRocket_next;
      CRocket *Rocket = (*iRocket);

      if (Rocket->m_Launcher == this) {
        Explode(Rocket->m_Pos.x, Rocket->m_Pos.y, m_Player);

        Rocket->m_Launcher->m_NumRockets--;

        std::list<CRocket *>::iterator iElement = iRocket;

        Game.m_RocketList.erase(iElement);
        delete Rocket;
      }
    }

    m_DumbFire = false;

    m_NumRockets = m_PowerLevel * 3;

    for (int i = 0; i < m_PowerLevel * 3; i++) {
      CRocket *Rocket;

      Rocket = new CRocket{};

      Rocket->m_Rot = m_Player->m_Rot;

      Rocket->m_Launcher = this;

      Rocket->m_Pos.x = m_HotPoint.x;
      Rocket->m_Pos.y = m_HotPoint.y;

      Rocket->m_Speed.x = (i - m_PowerLevel / 2.0f * 3.0f + 0.5f) * 3.0f;
      Rocket->m_Speed.y = 0.0f;

      Rocket->m_PowerLevel = m_PowerLevel;

      // Rocket->m_Rot=180.0f;

      Rocket->m_Fuel = W_MAX_ROCKET_FUEL;

      // m_NumRockets++;

      Game.m_RocketFlySound.PlaySound(&Rocket->SoundState);

      // Rocket->SoundState->SetVolume(-512,0);

      Game.m_RocketList.push_back(Rocket);
    }
  }
}

//---------------------------------------------------------------------------------
CRailgun::CRailgun() {
  m_FireDelay = 0.0f;
  m_PowerLevel = 1;

  m_WeaponType = WT_RAILGUN;
}

void CRailgun::Update() {
  if (m_FireDelay > 0.0f) {
    m_FireDelay -= 1.0f * XEngine->GetTween();
  }
}

void CRailgun::Fire() {
  Slug *MySlug;

  if (m_FireDelay <= 0.0f) {
    MySlug = new Slug{};
    MySlug->m_isIon = false;
    MySlug->m_Life = 60.0f;

    MySlug->m_PowerLevel = m_PowerLevel;

    MySlug->m_Pos.x = m_HotPoint.x;
    MySlug->m_Pos.y = m_HotPoint.y;
    MySlug->m_Speed.x = m_Player->m_AimerPos.x - MySlug->m_Pos.x;
    MySlug->m_Speed.y = m_Player->m_AimerPos.y - MySlug->m_Pos.y;

    float Theta = ATan2(MySlug->m_Speed.x, MySlug->m_Speed.y);

    MySlug->m_Speed.x = Sin(Theta) * 1600.0f;
    MySlug->m_Speed.y = Cos(Theta) * 1600.0f;

    m_FireDelay = 30.0f / m_PowerLevel;

    Game.m_SlugList.push_back(MySlug);

    Corona *MyCorona = new Corona{};

    MyCorona->m_Life = 40.0f;
    MyCorona->m_Rot = m_Player->m_Rot;

    MyCorona->m_Pos = m_HotPoint;

    Game.m_CoronaList.push_back(MyCorona);

    for (std::list<CBaby *>::iterator iBaby = Game.m_BabyList.begin(),
                                      iBaby_next = iBaby;
         iBaby != Game.m_BabyList.end(); iBaby = iBaby_next) {
      iBaby_next = iBaby;
      ++iBaby_next;
      CBaby *Baby = (*iBaby);
      COLLISIONINFO MyCollisionInfo = XEngine->Collide(
          MySlug->m_Pos.x, MySlug->m_Pos.y, 4.0f, MySlug->m_Speed.x,
          MySlug->m_Speed.y, Baby->m_Pos.x, Baby->m_Pos.y, 16.0f);

      if (MyCollisionInfo.Collided) {
        Baby->Damage(MySlug->m_Speed.x / 250.0f, MySlug->m_Speed.y / 250.0f, 8,
                     m_Player, 0);

        Spark *MySpark;

        for (float Theta = 0.0f; Theta < 360.0f; Theta += 10.0f) {
          MySpark = new Spark{};

          MySpark->m_Pos.x = Baby->m_Pos.x;
          MySpark->m_Pos.y = Baby->m_Pos.y;

          MySpark->m_Speed.x = Sin(Theta) * 8.0f;
          MySpark->m_Speed.y = Cos(Theta) * 8.0f;

          MySpark->m_Life = 30.0f;
          MySpark->m_Rot = Rand() * 360.0f;

          Game.m_SparkList.push_back(MySpark);
        }
      }
    }

    for (std::list<CWeaponBox *>::iterator iBox = Game.m_BoxList.begin(),
                                           iBox_next = iBox;
         iBox != Game.m_BoxList.end(); iBox = iBox_next) {
      iBox_next = iBox;
      ++iBox_next;
      CWeaponBox *WeaponBox = (*iBox);

      if (WeaponBox->m_BoxState == WBS_FLOATING) {
        COLLISIONINFO MyCollisionInfo = XEngine->Collide(
            MySlug->m_Pos.x, MySlug->m_Pos.y, 2.0f, MySlug->m_Speed.x,
            MySlug->m_Speed.y, WeaponBox->m_Pos.x, WeaponBox->m_Pos.y, 24.0f);

        if (MyCollisionInfo.Collided) {
          WeaponBox->m_BoxState = WBS_FALLING;

          Spark *MySpark;

          for (float Theta = 0.0f; Theta < 360.0f; Theta += 10.0f) {
            MySpark = new Spark{};

            MySpark->m_Pos.x = WeaponBox->m_Pos.x;
            MySpark->m_Pos.y = WeaponBox->m_Pos.y;

            MySpark->m_Speed.x = Sin(Theta) * 8.0f;
            MySpark->m_Speed.y = Cos(Theta) * 8.0f;

            MySpark->m_Life = 30.0f;
            MySpark->m_Rot = Rand() * 360.0f;

            Game.m_SparkList.push_back(MySpark);
          }
        }
      }
    }

    Game.m_RailgunSound.PlaySound();
  }
}

void CRailgun::FireAlternate() { Fire(); }

CMachineGun::CMachineGun() {
  m_Bullets = 30.0f;
  m_Reload = 0.0f;
  m_PowerLevel = 1;
  m_WeaponType = WT_MACHINEGUN;
  m_FlashTime = 10.0f;
  m_FlashType = 0;
}

void CMachineGun::Fire() {
  Bullet *MyBullet;

  if (m_Reload <= 0.0f && m_FireDelay <= 0.0f) {
    m_Bullets -= 0.1f * XEngine->GetTween();
    m_FireDelay = 0.5f * XEngine->GetTween();

    m_FlashTime = 0.0f;
    m_FlashType = rand() % 3;

    MyBullet = new Bullet{};

    MyBullet->m_Player = m_Player;

    MyBullet->m_Life = 30.0f;

    MyBullet->m_PowerLevel = m_PowerLevel;

    MyBullet->m_Pos.x = m_HotPoint.x;
    MyBullet->m_Pos.y = m_HotPoint.y;
    MyBullet->m_sPos.x = m_HotPoint.x;
    MyBullet->m_sPos.y = m_HotPoint.y;

    MyBullet->m_Speed.x = m_Player->m_AimerPos.x - MyBullet->m_Pos.x;
    MyBullet->m_Speed.y = m_Player->m_AimerPos.y - MyBullet->m_Pos.y;

    float Theta = ATan2(MyBullet->m_Speed.x, MyBullet->m_Speed.y);

    MyBullet->m_Speed.x = Sin(Theta + Rand() * 16.0f - 8.0f) * 32.0f;
    MyBullet->m_Speed.y = Cos(Theta + Rand() * 16.0f - 8.0f) * 32.0f;

    Game.m_BulletList.push_back(MyBullet);

    if (m_Bullets <= 0.0f) {
      m_Reload = 30.0f;
    }

    Game.m_MachinegunSound[rand() % 4].PlaySound();
  }
}

void CMachineGun::FireAlternate() {
  Bullet *MyBullet;

  if (m_Reload <= 0.0f) {
    m_FlashTime = -8.0f;
    m_FlashType = rand() % 3;

    for (int NumberofBullets = 0; NumberofBullets < 24; NumberofBullets++) {
      m_Bullets -= 1.0f;

      MyBullet = new Bullet{};

      MyBullet->m_Player = m_Player;

      MyBullet->m_Life = 30.0f;

      MyBullet->m_PowerLevel = m_PowerLevel;

      MyBullet->m_Pos.x = m_HotPoint.x;
      MyBullet->m_Pos.y = m_HotPoint.y;
      MyBullet->m_sPos.x = m_HotPoint.x;
      MyBullet->m_sPos.y = m_HotPoint.y;

      MyBullet->m_Speed.x = m_Player->m_AimerPos.x - MyBullet->m_Pos.x;
      MyBullet->m_Speed.y = m_Player->m_AimerPos.y - MyBullet->m_Pos.y;

      float Theta = ATan2(MyBullet->m_Speed.x, MyBullet->m_Speed.y);

      MyBullet->m_Speed.x = Sin(Theta + Rand() * 32.0f - 16.0f) * 32.0f;
      MyBullet->m_Speed.y = Cos(Theta + Rand() * 32.0f - 16.0f) * 32.0f;

      Game.m_BulletList.push_back(MyBullet);

      if (m_Bullets < 0.0f)
        break;
    }

    m_Reload = 20.0f;

    Game.m_ShotgunSound.PlaySound();
  }
}

void CMachineGun::Update() {
  m_FlashTime += 1.0f * XEngine->GetTween();

  if (m_FireDelay > 0.0f) {
    m_FireDelay -= 1.0f * XEngine->GetTween();
  }

  if (m_Reload > 0.0f) {
    m_Reload -= 1.0f * XEngine->GetTween();

    if (m_Reload <= 0.0f) {
      m_Bullets = 30.0f;
    }
  }
}

//-------------------------------------------------------------------------

CFlameThrower::CFlameThrower() {
  m_PowerLevel = 1;
  m_WeaponType = WT_FLAMETHROWER;
  m_Firing = false;
  m_FireDelay = 0.0f;
}

void CFlameThrower::Fire() {
  if (m_FireDelay <= 0.0f) {
    if (!m_Firing) {
      for (int i = 0; i < 6; i++) {
        Game.m_FlameThrowerRoastSound.PlaySound(&m_SoundState[i]);
      }

      Game.m_FlameThrowerFireSound.PlaySound();
    }

    m_Firing = true;
    m_FireDelay = 2.0f;

    // for (int i=0;i<m_PowerLevel;i++)
    //{
    Flame *MyFlame;

    MyFlame = new Flame{};

    MyFlame->m_Rot = Rand() * 360.0f;
    MyFlame->m_RotSpeed = Rand() * 8.0f - 4.0f;

    MyFlame->m_Launcher = this;
    MyFlame->m_Player = m_Player;

    MyFlame->m_Pos.x = m_HotPoint.x;
    MyFlame->m_Pos.y = m_HotPoint.y;

    float DX = m_Player->m_AimerPos.x - m_HotPoint.x;
    float DY = m_Player->m_AimerPos.y - m_HotPoint.y;

    float DL = Sqrt(DX * DX + DY * DY);

    MyFlame->m_Speed.x = DX / DL * 8.0f + Rand() - 0.5f;
    MyFlame->m_Speed.y = DY / DL * 8.0f + Rand() - 0.5f;

    MyFlame->m_PowerLevel = m_PowerLevel;

    MyFlame->m_Life = 20.0f + m_PowerLevel * 5.0f;

    Smoke *MySmoke;

    for (int i = 0; i < 10; i++) {
      MySmoke = new Smoke{};

      MySmoke->m_RocketSmoke = false;

      MySmoke->m_Pos.x = MyFlame->m_Pos.x;
      MySmoke->m_Pos.y = MyFlame->m_Pos.y;
      MySmoke->m_Speed.x = MyFlame->m_Speed.x * 0.5f + Rand() * 2.0f - 1.0f;
      MySmoke->m_Speed.y = MyFlame->m_Speed.y * 0.5f + Rand() * 2.0f - 1.0f;

      MySmoke->m_Life = 50.0f;
      MySmoke->m_Rot = Rand() * 360.0f;

      Game.m_SmokeList.push_back(MySmoke);
    }

    Game.m_FlameList.push_back(MyFlame);
  }
}

void CFlameThrower::Update() {
  if (XEngine->GamePad[m_Player->m_ControllerNum].bAnalogButtons[7] <= 75 &&
      m_Firing == true) {
    for (int i = 0; i < 6; i++) {
      XEngine->StopChannel(m_SoundState[i]);
    }

    m_Firing = false;
  }

  if (m_FireDelay >= 0.0f)
    m_FireDelay -= 1.0f * XEngine->GetTween();
}

void CFlameThrower::FireAlternate() {}

void CFlameThrower::Stop() {
  for (int i = 0; i < 6; i++) {
    XEngine->StopChannel(m_SoundState[i]);
  }
}

//----------------------------------------------------------------------------------------
// Player Code
//----------------------------------------------------------------------------------------

CBaby::CBaby() {
  m_LastHit = NULL;
  m_Killer = NULL;

  m_Speed.x = 0.0f;
  m_Speed.y = 0.0f;

  m_Rot = 0.0f;
  m_DeltaRot = 0.0f;
  m_Stand = 240.0f;

  m_NumBalloons = 3;
  m_ThreatLevel = 0;
  m_Health = 32;

  m_HasBomb = false;
  m_OnFire = false;

  m_Hit = 0;

  if (Rand() > 0.5f) {
    m_BabyState = BS_FLOATINGLEFT;
    m_Pos.x = 640.0f + 32.0f;

  } else {
    m_BabyState = BS_FLOATINGRIGHT;
    m_Pos.x = 0.0f - 32.0f;
  }

  m_Pos.y = 80.0f + Rand() * 280.0f;
}

void CBaby::Damage(float VX, float VY, int NumDamage, CPlayer *Player,
                   int damageType) {
  if (m_Hit <= 0 || damageType != 1) {
    if (m_BabyState == BS_FLOATINGLEFT || m_BabyState == BS_FLOATINGRIGHT) {
      int BalloonsLost = NumDamage;
      if (BalloonsLost > m_NumBalloons)
        BalloonsLost = m_NumBalloons;

      Balloon *MyBalloon;

      for (int b = 0; b < BalloonsLost; b++) {
        MyBalloon = new Balloon{};

        MyBalloon->m_Pos.x = m_Pos.x;
        MyBalloon->m_Pos.y = m_Pos.y;

        MyBalloon->m_Speed.x = Rand() * 2.0f - 1.0f;
        MyBalloon->m_Speed.y = Rand() - 0.5f;

        Game.m_BalloonList.push_back(MyBalloon);
      }

      m_NumBalloons -= NumDamage;

      if (m_NumBalloons <= 0) {
        Game.m_BabiesKilled++;
        Game.m_NumBabies--;

        m_ThreatLevel = 0;
        m_BabyState = BS_BOUNCING;
        m_Combo = -1;

        if (m_HasBomb == true) {
          m_HasBomb = false;

          CBomb *MyBomb;

          MyBomb = new CBomb{};

          MyBomb->m_Size = (float)m_ThreatLevel;
          MyBomb->m_Pos.y = m_Pos.y + 24.0f;
          MyBomb->m_Pos.x = m_Pos.x;
          MyBomb->m_Speed.y = 0.0f;

          if (m_BabyState == BS_FLOATINGLEFT) {
            MyBomb->m_Speed.x = +G_BABY_FLOAT_SPEED;
          } else {
            MyBomb->m_Speed.x = -G_BABY_FLOAT_SPEED;
          }

          Game.m_BombList.push_back(MyBomb);
        }

        Player->AddScore(100, m_Pos.x, m_Pos.y);
      }
    }
  }

  if (m_BabyState == BS_BOUNCING) {
    if (m_Pos.y < 430.0f) {
      m_Combo++;
    }

    m_LastHit = Player;

    /*
    if (m_Combo>0 && (m_Combo % 10 == 0))
    {
            Player->AddScore(m_Combo*10,m_Pos.x,m_Pos.y);
    }
    */

    //	m_Hit=2;

    m_Speed.x += VX;
    m_Speed.y += VY;

    m_DeltaRot = Rand() * 50.0f;
  }

  // if (m_BabyState==BS_WALKLEFT || m_BabyState==BS_WALKRIGHT)
  //{
  if (m_Hit <= 0 || damageType != 1) {
    m_Health -= NumDamage;
    m_Hit = 2;
  }

  if (m_Health <= 0) {
    m_Killer = Player;
  }
  //}
}

CWeaponBox::CWeaponBox() {
  if (rand() % 2 == 0) {
    m_Speed.x = -1.0f;
    m_Pos.x = 640 + 32.0f;
  } else {
    m_Speed.x = 1.0f;
    m_Pos.x = -32.0f;
  }

  m_Speed.y = 0.0f;
  m_Pos.y = 80.0f + Rand() * 280.0f;

  m_Life = 600.0f;
  m_Rot = 0.0f;

  if (rand() % 4 == 0) {
    m_Special = true;
    m_WeaponType = rand() % W_NUM_SPECIALS;
  } else {
    m_Special = false;
    m_WeaponType = rand() % W_NUM_WEAPONS;
    if (const char *test = getenv("DBX_TEST_WEAPON")) {
      int n = atoi(test);
      if (n >= 1 && n <= 6)
        m_WeaponType = n;
    }
  }

  m_BoxState = WBS_FLOATING;
}

CPlayer::CPlayer() {
  m_PlayerCondition = PC_DEAD;
  m_ConditionTime = -1.0f;

  m_Score = 0;
  if (const char *test = getenv("DBX_TEST_SCORE"))
    m_Score = atoi(test);
  m_Lives = 4;

  m_Weapon = NULL;

  Game.m_NumPlayers++;
}

void CPlayer::AddScore(int NewScore, float X, float Y) {
  m_Bonus += NewScore; // crash on this line
  m_Score += m_Bonus;

  Score *MyScore = new Score{};

  MyScore->m_Pos.x = X;
  MyScore->m_Pos.y = Y;

  MyScore->m_Value = m_Bonus;
  MyScore->m_Life = 60.0f;

  switch (m_ControllerNum) {
  case 0:

    MyScore->m_Red = 255;
    MyScore->m_Blue = 0;
    MyScore->m_Green = 0;

    break;

  case 1:

    MyScore->m_Red = 255;
    MyScore->m_Blue = 0;
    MyScore->m_Green = 255;

    break;

  case 2:

    MyScore->m_Red = 0;
    MyScore->m_Blue = 255;
    MyScore->m_Green = 0;

    break;

  case 3:

    MyScore->m_Red = 0;
    MyScore->m_Blue = 0;
    MyScore->m_Green = 255;

    break;
  }

  Game.m_ScoreList.push_back(MyScore);
}

void CPlayer::Init() {
  if (m_Lives >= 0) {
    m_Lives--;

    if (m_Weapon != NULL) {
      // delete m_Weapon;
    }

    m_WhiteSpecial = NULL;
    m_BlackSpecial = NULL;

    int WeaponType = rand() % (W_NUM_WEAPONS - 1) + 1;
    if (const char *test = getenv("DBX_TEST_WEAPON")) {
      int n = atoi(test);
      if (n >= 1 && n <= 6)
        WeaponType = n;
    }

    switch (WeaponType) {
    case WT_ROCKETS:

      m_Weapon = OwnWeapon<CRockets>();
      break;

    case WT_RAILGUN:

      m_Weapon = OwnWeapon<CRailgun>();
      break;

    case WT_MACHINEGUN:

      m_Weapon = OwnWeapon<CMachineGun>();
      break;

    case WT_PLASMA:

      m_Weapon = OwnWeapon<CPlasma>();
      break;

    case WT_GRENADES:

      m_Weapon = OwnWeapon<CGrenades>();
      break;

    case WT_FLAMETHROWER:

      m_Weapon = OwnWeapon<CFlameThrower>();
      break;

    default:

      m_Weapon = OwnWeapon<CMachineGun>();
      break;
    }

    m_Weapon->m_Player = this;

    m_AimerPos.x = m_ControllerNum * 160.0f + 10.0f;
    m_AimerPos.y = 60.0f;

    m_PlayerPos.x = m_AimerPos.x;
    m_PlayerPos.y = 440.0f;

    m_PlayerCondition = PC_INVINCIBLE;
    m_ConditionTime = 300.0f;
  }
}

void CPlayer::Update() {}

void CPlayer::Shutdown() {}

void CGame::LoadArt() {
  // load some graphics
  m_BGImage[0] = XEngine->LoadTexture("d:\\data\\backgrounds\\train_fg.png");
  m_BGImage[1] = XEngine->LoadTexture("d:\\data\\backgrounds\\train_bg1.png");
  m_BGImage[2] = XEngine->LoadTexture("d:\\data\\backgrounds\\train_bg2.png");
  m_BGImage[3] = XEngine->LoadTexture("d:\\data\\backgrounds\\train_bg3.png");

  m_AimerImage[0] = XEngine->LoadTexture("d:\\data\\aimers\\aimer_red.png");
  m_AimerImage[1] = XEngine->LoadTexture("d:\\data\\aimers\\aimer_yellow.png");
  m_AimerImage[2] = XEngine->LoadTexture("d:\\data\\aimers\\aimer_blue.png");
  m_AimerImage[3] = XEngine->LoadTexture("d:\\data\\aimers\\aimer_green.png");

  m_WeaponParachuteImage =
      XEngine->LoadTexture("d:\\data\\crates\\Paracute.png");

  m_WeaponBoxImage[0] =
      XEngine->LoadTexture("d:\\data\\crates\\Random_crate.png");
  m_WeaponBoxImage[1] =
      XEngine->LoadTexture("d:\\data\\crates\\Rail_crate.png");
  m_WeaponBoxImage[2] = XEngine->LoadTexture("d:\\data\\crates\\Gun_crate.png");
  m_WeaponBoxImage[3] =
      XEngine->LoadTexture("d:\\data\\crates\\Rocket_crate.png");
  m_WeaponBoxImage[4] =
      XEngine->LoadTexture("d:\\data\\crates\\plasma_crate.png");
  m_WeaponBoxImage[5] =
      XEngine->LoadTexture("d:\\data\\crates\\grenade_crate.png");

  m_WeaponBoxImage[WT_FLAMETHROWER] =
      XEngine->LoadTexture("d:\\data\\crates\\flamethrower_crate.png");

  m_SpecialBoxImage[0] =
      XEngine->LoadTexture("d:\\data\\crates\\nuke_crate.png");
  m_SpecialBoxImage[1] =
      XEngine->LoadTexture("d:\\data\\crates\\shield_crate.png");
  m_SpecialBoxImage[2] =
      XEngine->LoadTexture("d:\\data\\crates\\gravity_crate.png");
  m_SpecialBoxImage[3] =
      XEngine->LoadTexture("d:\\data\\crates\\time_crate.png");
  m_SpecialBoxImage[4] =
      XEngine->LoadTexture("d:\\data\\crates\\ioncannon_crate.png");

  m_PlayerImage[0][0] =
      XEngine->LoadTexture("d:\\data\\player\\Red\\Red_Legs_1.png");
  m_PlayerImage[0][1] =
      XEngine->LoadTexture("d:\\data\\player\\Red\\Red_Legs_2.png");
  m_PlayerImage[0][2] =
      XEngine->LoadTexture("d:\\data\\player\\Red\\Red_Legs_3.png");
  m_PlayerImage[0][3] =
      XEngine->LoadTexture("d:\\data\\player\\Red\\Red_Legs_4.png");
  m_PlayerImage[0][4] =
      XEngine->LoadTexture("d:\\data\\player\\Red\\Red_Legs_Stand.png");
  m_PlayerImage[0][5] =
      XEngine->LoadTexture("d:\\data\\player\\Red\\Red_0.png");
  m_PlayerImage[0][6] =
      XEngine->LoadTexture("d:\\data\\player\\Red\\Red_45.png");
  m_PlayerImage[0][7] =
      XEngine->LoadTexture("d:\\data\\player\\Red\\Red_90.png");

  m_PlayerImage[1][0] =
      XEngine->LoadTexture("d:\\data\\player\\Yellow\\Yellow_Legs_1.png");
  m_PlayerImage[1][1] =
      XEngine->LoadTexture("d:\\data\\player\\Yellow\\Yellow_Legs_2.png");
  m_PlayerImage[1][2] =
      XEngine->LoadTexture("d:\\data\\player\\Yellow\\Yellow_Legs_3.png");
  m_PlayerImage[1][3] =
      XEngine->LoadTexture("d:\\data\\player\\Yellow\\Yellow_Legs_4.png");
  m_PlayerImage[1][4] =
      XEngine->LoadTexture("d:\\data\\player\\Yellow\\Yellow_Legs_Stand.png");
  m_PlayerImage[1][5] =
      XEngine->LoadTexture("d:\\data\\player\\Yellow\\Yellow_0.png");
  m_PlayerImage[1][6] =
      XEngine->LoadTexture("d:\\data\\player\\Yellow\\Yellow_45.png");
  m_PlayerImage[1][7] =
      XEngine->LoadTexture("d:\\data\\player\\Yellow\\Yellow_90.png");

  m_PlayerImage[2][0] =
      XEngine->LoadTexture("d:\\data\\player\\Blue\\Blue_Legs_1.png");
  m_PlayerImage[2][1] =
      XEngine->LoadTexture("d:\\data\\player\\Blue\\Blue_Legs_2.png");
  m_PlayerImage[2][2] =
      XEngine->LoadTexture("d:\\data\\player\\Blue\\Blue_Legs_3.png");
  m_PlayerImage[2][3] =
      XEngine->LoadTexture("d:\\data\\player\\Blue\\Blue_Legs_4.png");
  m_PlayerImage[2][4] =
      XEngine->LoadTexture("d:\\data\\player\\Blue\\Blue_Legs_Stand.png");
  m_PlayerImage[2][5] =
      XEngine->LoadTexture("d:\\data\\player\\Blue\\Blue_0.png");
  m_PlayerImage[2][6] =
      XEngine->LoadTexture("d:\\data\\player\\Blue\\Blue_45.png");
  m_PlayerImage[2][7] =
      XEngine->LoadTexture("d:\\data\\player\\Blue\\Blue_90.png");

  m_PlayerImage[3][0] =
      XEngine->LoadTexture("d:\\data\\player\\Green\\Green_Legs_1.png");
  m_PlayerImage[3][1] =
      XEngine->LoadTexture("d:\\data\\player\\Green\\Green_Legs_2.png");
  m_PlayerImage[3][2] =
      XEngine->LoadTexture("d:\\data\\player\\Green\\Green_Legs_3.png");
  m_PlayerImage[3][3] =
      XEngine->LoadTexture("d:\\data\\player\\Green\\Green_Legs_4.png");
  m_PlayerImage[3][4] =
      XEngine->LoadTexture("d:\\data\\player\\Green\\Green_Legs_Stand.png");
  m_PlayerImage[3][5] =
      XEngine->LoadTexture("d:\\data\\player\\Green\\Green_0.png");
  m_PlayerImage[3][6] =
      XEngine->LoadTexture("d:\\data\\player\\Green\\Green_45.png");
  m_PlayerImage[3][7] =
      XEngine->LoadTexture("d:\\data\\player\\Green\\Green_90.png");

  m_PlayerLivesImage[0] =
      XEngine->LoadTexture("d:\\data\\player\\red\\red_lives.png");
  m_PlayerLivesImage[1] =
      XEngine->LoadTexture("d:\\data\\player\\yellow\\yellow_lives.png");
  m_PlayerLivesImage[2] =
      XEngine->LoadTexture("d:\\data\\player\\blue\\blue_lives.png");
  m_PlayerLivesImage[3] =
      XEngine->LoadTexture("d:\\data\\player\\green\\green_lives.png");

  m_GUIBoxImage = XEngine->LoadTexture("d:\\data\\gui\\gui_box.png");

  m_BabyFloatingImage[0] =
      XEngine->LoadTexture("d:\\data\\babies\\babyballoon1.png");
  m_BabyFloatingImage[1] =
      XEngine->LoadTexture("d:\\data\\babies\\babyballoon2.png");
  m_BabyFloatingImage[2] =
      XEngine->LoadTexture("d:\\data\\babies\\babyballoon3.png");

  m_BabyGetUpImage[0] =
      XEngine->LoadTexture("d:\\data\\babies\\Baby_GetUp_1.png");
  m_BabyGetUpImage[1] =
      XEngine->LoadTexture("d:\\data\\babies\\Baby_GetUp_2.png");
  m_BabyGetUpImage[2] =
      XEngine->LoadTexture("d:\\data\\babies\\Baby_GetUp_3.png");
  m_BabyGetUpImage[3] =
      XEngine->LoadTexture("d:\\data\\babies\\Baby_GetUp_4.png");

  m_BabyWalkImage[0] =
      XEngine->LoadTexture("d:\\data\\babies\\Baby_Walk_1.png");
  m_BabyWalkImage[1] =
      XEngine->LoadTexture("d:\\data\\babies\\Baby_Walk_2.png");
  m_BabyWalkImage[2] =
      XEngine->LoadTexture("d:\\data\\babies\\Baby_Walk_3.png");
  m_BabyWalkImage[3] =
      XEngine->LoadTexture("d:\\data\\babies\\Baby_Walk_4.png");
  m_BabyWalkImage[4] =
      XEngine->LoadTexture("d:\\data\\babies\\Baby_Walk_5.png");

  m_BabyDeadImage = XEngine->LoadTexture("d:\\data\\babies\\deadbaby.png");
  m_BalloonImage = XEngine->LoadTexture("d:\\data\\babies\\balloon.png");

  m_GibImage[0] = XEngine->LoadTexture("d:\\data\\gibs\\gib1.png");
  m_GibImage[1] = XEngine->LoadTexture("d:\\data\\gibs\\gib2.png");
  m_GibImage[2] = XEngine->LoadTexture("d:\\data\\gibs\\gib3.png");
  m_GibImage[3] = XEngine->LoadTexture("d:\\data\\gibs\\gib4.png");
  m_GibImage[4] = XEngine->LoadTexture("d:\\data\\gibs\\gib5.png");
  m_GibImage[5] = XEngine->LoadTexture("d:\\data\\gibs\\gib6.png");

  m_RocketImage = XEngine->LoadTexture("d:\\data\\weapons\\rocket.png");
  m_SmokeImage = XEngine->LoadTexture("d:\\data\\weapons\\smoke.png");
  m_RocketSmokeImage =
      XEngine->LoadTexture("d:\\data\\weapons\\rocket_smoke.png");
  m_FireImage = XEngine->LoadTexture("d:\\data\\weapons\\fire.png");
  m_PlasmaImage = XEngine->LoadTexture("d:\\data\\weapons\\plasma.png");

  m_BulletTracerImage =
      XEngine->LoadTexture("d:\\data\\weapons\\bullet_tracer.png");

  m_MuzzleFlashImage[0] =
      XEngine->LoadTexture("d:\\data\\weapons\\muzzle_flash_1.png");
  m_MuzzleFlashImage[1] =
      XEngine->LoadTexture("d:\\data\\weapons\\muzzle_flash_2.png");
  m_MuzzleFlashImage[2] =
      XEngine->LoadTexture("d:\\data\\weapons\\muzzle_flash_3.png");

  m_SlugTracerImage =
      XEngine->LoadTexture("d:\\data\\weapons\\rail_tracer.png");
  m_CoronaImage = XEngine->LoadTexture("d:\\data\\weapons\\corona.png");

  m_SparkImage = XEngine->LoadTexture("d:\\data\\weapons\\spark.png");
  m_BombImage = XEngine->LoadTexture("d:\\data\\babies\\Baby_Bomb.png");

  m_PowerBarImage = XEngine->LoadTexture("d:\\data\\weapons\\power_bar.png");
  m_PowerBarBoxImage =
      XEngine->LoadTexture("d:\\data\\weapons\\power_bar_box.png");
  m_GameOverImage = XEngine->LoadTexture("d:\\data\\gui\\game_over.png");

  m_GrenadeImage = XEngine->LoadTexture("d:\\data\\weapons\\grenade.png");
}

void CGame::Init() {
  srand(LanActive()?LanSeed():XEngine->m_Millisecs.LowPart);

  m_BabyList.clear();
  m_RocketList.clear();
  m_BoxList.clear();
  m_SpecialList.clear();

  //<temp>
  m_Level = new CLevel{};
  m_Level->m_FloorMaterial = FM_METAL;

  // DSI3DL2LISTENER MyAudioEnvironment={DSI3DL2_ENVIRONMENT_PRESET_GENERIC};
  DSI3DL2LISTENER MyAudioEnvironment = {DSI3DL2_ENVIRONMENT_PRESET_NOREVERB};
  // DSI3DL2LISTENER MyAudioEnvironment={DSI3DL2_ENVIRONMENT_PRESET_SEWERPIPE};

  XEngine->SetI3DL2Listener(&MyAudioEnvironment);

  //</temp>

  LoadArt();

  // sound and music

  m_FlameThrowerRoastSound.Create(
      "d:\\data\\sounds\\flamethrower\\ft_stream.wav");
  m_FlameThrowerRoastSound.SetSoundType(ST_LOOPING);

  m_FlameThrowerFireSound.Create("d:\\data\\sounds\\flamethrower\\ft_fire.wav");
  m_FlameThrowerHitSound.Create("d:\\data\\sounds\\flamethrower\\ft_hit.wav");

  m_ShotgunSound.Create("d:\\data\\sounds\\shotgun\\sshotf1b.wav");

  m_MachinegunSound[0].Create("d:\\data\\sounds\\machinegun\\machinegun1.wav");
  m_MachinegunSound[1].Create("d:\\data\\sounds\\machinegun\\machinegun2.wav");
  m_MachinegunSound[2].Create("d:\\data\\sounds\\machinegun\\machinegun3.wav");
  m_MachinegunSound[3].Create("d:\\data\\sounds\\machinegun\\machinegun4.wav");

  m_BulletHitSound[0].Create("d:\\data\\sounds\\machinegun\\hit1.wav");
  m_BulletHitSound[1].Create("d:\\data\\sounds\\machinegun\\hit2.wav");
  m_BulletHitSound[2].Create("d:\\data\\sounds\\machinegun\\hit3.wav");

  m_PlasmaLaunchSound.Create("d:\\data\\sounds\\plasma\\launch.wav");
  m_PlasmaHitSound.Create("d:\\data\\sounds\\plasma\\hit.wav");

  m_RocketLaunchSound.Create("d:\\data\\sounds\\rocket\\rocketlaunch.wav");
  m_RocketExplodeSound.Create("d:\\data\\sounds\\rocket\\rocketexplode.wav");
  m_RocketFlySound.Create("d:\\data\\sounds\\rocket\\rocketfly.wav");
  m_RocketFlySound.SetSoundType(ST_LOOPING);

  m_RailgunSound.Create("d:\\data\\sounds\\railgun\\railgun.wav");

  m_WeaponUpgradeSound.Create("d:\\data\\sounds\\pickups\\weaponupgrade.wav");
  m_WeaponPickupSound.Create(
      "d:\\data\\sounds\\pickups\\weaponupgrade.wav"); // temp

  m_StepSound[0].Create("d:\\data\\sounds\\steps\\boot1.wav");
  m_StepSound[1].Create("d:\\data\\sounds\\steps\\boot2.wav");
  m_StepSound[2].Create("d:\\data\\sounds\\steps\\boot3.wav");
  m_StepSound[3].Create("d:\\data\\sounds\\steps\\boot4.wav");
  m_StepSound[4].Create("d:\\data\\sounds\\steps\\flesh1.wav");
  m_StepSound[5].Create("d:\\data\\sounds\\steps\\flesh2.wav");
  m_StepSound[6].Create("d:\\data\\sounds\\steps\\flesh3.wav");
  m_StepSound[7].Create("d:\\data\\sounds\\steps\\flesh4.wav");

  m_GrenadeLaunchSound.Create("d:\\data\\sounds\\grenade\\launch.wav");
  m_GrenadeBounceSound[0].Create("d:\\data\\sounds\\grenade\\bounce1.wav");
  m_GrenadeBounceSound[1].Create("d:\\data\\sounds\\grenade\\bounce2.wav");

  // should be in level class
  m_AmbientSound.Create("d:\\data\\sounds\\ambient\\machinerydrone01.wav");
  m_AmbientSound.SetSoundType(ST_LOOPING);
  m_AmbientSound.PlaySound();

  switch (m_Level->m_FloorMaterial) {
  case FM_GENERIC:

    m_FloorSound[0].Create("d:\\data\\sounds\\steps\\step1.wav");
    m_FloorSound[1].Create("d:\\data\\sounds\\steps\\step2.wav");
    m_FloorSound[2].Create("d:\\data\\sounds\\steps\\step3.wav");
    m_FloorSound[3].Create("d:\\data\\sounds\\steps\\step4.wav");

    break;

  case FM_METAL:

    m_FloorSound[0].Create("d:\\data\\sounds\\steps\\clank1.wav");
    m_FloorSound[1].Create("d:\\data\\sounds\\steps\\clank2.wav");
    m_FloorSound[2].Create("d:\\data\\sounds\\steps\\clank3.wav");
    m_FloorSound[3].Create("d:\\data\\sounds\\steps\\clank4.wav");

    break;

  case FM_WATER:

    m_FloorSound[0].Create("d:\\data\\sounds\\steps\\splash1.wav");
    m_FloorSound[1].Create("d:\\data\\sounds\\steps\\splash2.wav");
    m_FloorSound[2].Create("d:\\data\\sounds\\steps\\splash3.wav");
    m_FloorSound[3].Create("d:\\data\\sounds\\steps\\splash4.wav");

    break;
  }

  m_GibSound[0].Create("d:\\data\\sounds\\gib\\bounce1.wav");
  m_GibSound[1].Create("d:\\data\\sounds\\gib\\bounce2.wav");
  m_GibSound[2].Create("d:\\data\\sounds\\gib\\bounce3.wav");
  m_GibSound[3].Create("d:\\data\\sounds\\gib\\gibbed.wav");

  XEngine->PlayMusic("d:\\data\\music\\music3.wma");
  // XEngine->PauseMusic();

  m_Gravity = 1.0f;

  m_BabiesKilled = 4;
  m_NumBabies = 0;

  m_NumPlayers = 0;

  for (int i = 0; i < 4; i++) {
    m_BestScores[i] = 0;
    m_Continues[i] = 0;
  }

  m_BabyRate = 50.0f;
  m_BabyDelay = 60.0f * 4.0f;
  m_GameOverDelay = 300.0f;

  m_BoxDelay = 420.0f + Rand() * 420.0f;

  m_ScrollSpeed = 0.0f;
  m_Scroll = 0.0f;

  int initialPlayers = startingPlayers;
  if (const char *test = getenv("DBX_TEST_PLAYERS"))
    initialPlayers = std::clamp(atoi(test), 1, 4);
  for (int NumPlayers = 0; NumPlayers < initialPlayers; NumPlayers++) {
    CPlayer *MyPlayer;

    MyPlayer = OwnPlayer();

    // temp
    MyPlayer->m_PlayerProfile = new PROFILE{};

    MyPlayer->m_PlayerProfile->JoystickSensitivity.x = 10.0f;
    MyPlayer->m_PlayerProfile->JoystickSensitivity.y = 10.0f;

    MyPlayer->m_ControllerNum = NumPlayers;

    MyPlayer->Init();

    m_PlayerList.push_back(MyPlayer);
  }

  fprintf(stderr, "GAME players started: %d\n", m_NumPlayers);
  fprintf(stderr, "GAME weapon level cap: %d\n", WeaponPowerLimit());
  if (getenv("DBX_TEST_PICKUPS") && !m_PlayerList.empty()) {
    auto *player = m_PlayerList.front();
    for (int i = 0; i < 8; ++i) {
      auto *box = new CWeaponBox{};
      box->m_Special = false;
      box->m_WeaponType = player->m_Weapon->m_WeaponType;
      box->m_Pos = player->m_PlayerPos;
      box->m_Speed = {0, 0};
      box->m_BoxState = WBS_FALLING;
      m_BoxList.push_back(box);
    }
  }
  for (int i = 0; i < 10; i++) {
    XEngine->XUpdate();
  }
}

int CGame::Update() {
  // Deterministic end-of-round fixture; absent from ordinary play.
  if (const char *test = getenv("DBX_TEST_GAMEOVER_AT")) {
    if (!m_ResultsReady && XEngine->m_Ticks >= atof(test) * 60) {
      for (auto *p : m_PlayerList) p->m_Lives = -1;
    }
  }
  if (m_NumPlayers <= 0) {
    if (!m_ResultsReady) FinishRound();
    if(LanActive())m_ResultsTime += XEngine->GetTween();
    else m_ResultsTime = float(GetTickCount64() - m_ResultsStarted) * .06f;
    if (!m_ResultWinnerRevealed && !m_ResultOrder.empty() &&
        m_ResultsTime / 60.f >= NativeResultRollDuration(m_ResultOrder.size())) {
      m_ResultWinnerRevealed = true;
      XEngine->PlayResultCue(true);
      int winner = m_ResultOrder.front();
      bool tied = m_ResultOrder.size() > 1 && m_FinalScores[m_ResultOrder[1]] == m_FinalScores[winner];
      fprintf(stderr, "RESULTS winner reveal player=%d score=%d tied=%d\n", winner+1, m_FinalScores[winner], tied);
    }
    if (NativeResultsCanContinue(m_ResultOrder.size(), m_ResultsTime / 60.f)) {
      for (const auto &pad : XEngine->GamePad)
        if ((pad.wPressedButtons & 0x10) || pad.bPressedAnalogButtons[0])
          return 0;
    }
    Refresh();
    XEngine->XUpdate();
    return 1;
  }

  for (int i = 0; i < 4; i++) {
    if (XEngine->GamePad[i].wPressedButtons & 0x00000010) {
      bool AlreadyExists = false;

      for (std::list<CPlayer *>::iterator iPlayer = m_PlayerList.begin(),
                                          iPlayer_next = iPlayer;
           iPlayer != m_PlayerList.end(); iPlayer = iPlayer_next) {
        iPlayer_next = iPlayer;
        ++iPlayer_next;
        CPlayer *Player = (*iPlayer);

        if (Player->m_ControllerNum == i) {
          AlreadyExists = true;

          break;
        }
      }

      if (AlreadyExists == true) {
        m_Paused = !m_Paused;
      }
    }
  }

  if (m_Paused == false) {
    if (m_NumPlayers <= 0) {
      m_GameOverDelay -= 1.0f * XEngine->GetTween();
    }

    if (m_GameOverDelay < 0.0f) {
      for (int m_GamePadButton = 0; m_GamePadButton < 3; m_GamePadButton++) {
        if (XEngine->GamePad[m_GamePadButton].wPressedButtons) {

          // deinitialize
          return 0;
        }
      }
    }

    // create some babies!!!!
    m_BabyDelay -= 1.0f * XEngine->GetTween();
    m_BoxDelay -= 1.0f * XEngine->GetTween();

    m_BabyRate = 9000.0f / (m_BabiesKilled + 300.0f);

    if (m_BabyDelay < 0.0f &&
        m_NumBabies < m_BabiesKilled / (5 - m_NumPlayers)) {
      m_BabyDelay = m_BabyRate;

      CBaby *MyBaby;
      MyBaby = new CBaby{};

      m_BabyList.push_back(MyBaby);

      m_NumBabies++;
    }

    if (m_BoxDelay < 0.0f) {
      m_BoxDelay = (600.0f + Rand() * 600.0f) * 8.0f / float(W_NUM_WEAPONS) /
                   float(W_NUM_SPECIALS);

      CWeaponBox *WeaponBox;

      WeaponBox = new CWeaponBox{};
      m_BoxList.push_back(WeaponBox);
    }

    if (m_ScrollSpeed < 24.0f) {
      m_ScrollSpeed += 0.01f * XEngine->GetTween();
    }

    m_Scroll -= m_ScrollSpeed * XEngine->GetTween();

    for (int i = 0; i < 4; i++) {
      if (XEngine->GamePad[i].wPressedButtons & 0x00000010) {
        bool AlreadyExists = false;

        for (std::list<CPlayer *>::iterator iPlayer = m_PlayerList.begin(),
                                            iPlayer_next = iPlayer;
             iPlayer != m_PlayerList.end(); iPlayer = iPlayer_next) {
          iPlayer_next = iPlayer;
          ++iPlayer_next;
          CPlayer *Player = (*iPlayer);

          if (Player->m_ControllerNum == i) {
            AlreadyExists = true;
            break;
          }
        }

        if (AlreadyExists == false && m_Continues[i] >= 0) {
          CPlayer *MyPlayer;

          MyPlayer = OwnPlayer();

          MyPlayer->m_PlayerProfile = new PROFILE{};

          MyPlayer->m_PlayerProfile->JoystickSensitivity.x = 8.0f;
          MyPlayer->m_PlayerProfile->JoystickSensitivity.y = 8.0f;

          MyPlayer->m_ControllerNum = i;

          MyPlayer->Init();

          m_PlayerList.push_back(MyPlayer);
        }
      }
    }

    for (std::list<CPlayer *>::iterator iPlayer = Game.m_PlayerList.begin(),
                                        iPlayer_next = iPlayer;
         iPlayer != Game.m_PlayerList.end(); iPlayer = iPlayer_next) {
      iPlayer_next = iPlayer;
      ++iPlayer_next;
      CPlayer *Player = (*iPlayer);

      Player->m_Bonus = 0;
    }

    for (std::list<CBaby *>::iterator iBaby = m_BabyList.begin(),
                                      iBaby_next = iBaby;
         iBaby != m_BabyList.end(); iBaby = iBaby_next) {
      iBaby_next = iBaby;
      ++iBaby_next;
      CBaby *Baby = (*iBaby);

      if (Baby->m_Hit > 0) {
        Baby->m_Hit--;
      }
    }

    for (std::list<CWeaponBox *>::iterator iWeaponBox = m_BoxList.begin(),
                                           iWeaponBox_next = iWeaponBox;
         iWeaponBox != m_BoxList.end(); iWeaponBox = iWeaponBox_next) {
      iWeaponBox_next = iWeaponBox;
      ++iWeaponBox_next;
      CWeaponBox *WeaponBox = (*iWeaponBox);

      WeaponBox->m_Rot += 2.0f * XEngine->GetTween();

      switch (WeaponBox->m_BoxState) {
      case WBS_FLOATING:

        WeaponBox->m_Pos.y +=
            Sin(WeaponBox->m_Rot) / 2.0f * XEngine->GetTween();

        break;

      case WBS_FALLING:

        WeaponBox->m_Speed.y += 0.1f * XEngine->GetTween() * m_Gravity;

        if (WeaponBox->m_Pos.y > 450.0f) {
          WeaponBox->m_Speed.y = -WeaponBox->m_Speed.y * .5f;
          WeaponBox->m_Pos.y = 450.0f;

          if (WeaponBox->m_Speed.y > -.55f) {
            WeaponBox->m_Speed.x = 0.0f;
          }
        }

        WeaponBox->m_Life -= 1.0f * XEngine->GetTween();

        break;
      };

      WeaponBox->m_Pos.x += WeaponBox->m_Speed.x * XEngine->GetTween();
      WeaponBox->m_Pos.y += WeaponBox->m_Speed.y * XEngine->GetTween();

      if (WeaponBox->m_Pos.x < -32.0f || WeaponBox->m_Pos.x > 640.0f + 32.0f ||
          WeaponBox->m_Life <= 0.0f) {
        std::list<CWeaponBox *>::iterator iElement = iWeaponBox;

        m_BoxList.erase(iElement);
        delete WeaponBox;
      }
    }

    for (std::list<CPlayer *>::iterator iPlayer = m_PlayerList.begin(),
                                        iPlayer_next = iPlayer;
         iPlayer != m_PlayerList.end(); iPlayer = iPlayer_next) {
      iPlayer_next = iPlayer;
      ++iPlayer_next;
      CPlayer *Player = (*iPlayer);

      Player->m_ConditionTime -= 1.0f * XEngine->GetTween();

      if (Player->m_ConditionTime <= 0.0f) {
        if (Player->m_PlayerCondition == PC_DEAD) {
          Player->Init();
        } else if (Player->m_PlayerCondition == PC_INVINCIBLE) {
          Player->m_PlayerCondition = PC_ALIVE;
        }
      }

      if (Player->m_PlayerCondition != PC_DEAD) {
        Player->m_AimerPos.x += XEngine->GamePad[Player->m_ControllerNum].fX2 *
                                Player->m_PlayerProfile->JoystickSensitivity.x *
                                XEngine->GetTween() / XEngine->m_GameSpeed;
        Player->m_AimerPos.y -= XEngine->GamePad[Player->m_ControllerNum].fY2 *
                                Player->m_PlayerProfile->JoystickSensitivity.y *
                                XEngine->GetTween() / XEngine->m_GameSpeed;

        float Joy1X = XEngine->GamePad[Player->m_ControllerNum].fX1,
              Joy1Y = XEngine->GamePad[Player->m_ControllerNum].fY1;

        if (Joy1X > 0.25f) {
          // walk right
          Player->m_PlayerPos.x += 3.0f * XEngine->GetTween();
          Player->m_PlayerState = PS_RUNRIGHT;

          if (Player->m_Direction < 0) {
            Player->m_FrameTime -= 0.25f * XEngine->GetTween();
          } else {
            Player->m_FrameTime += 0.25f * XEngine->GetTween();
          }
        } else if (Joy1X < -0.25f) {
          // walk left
          Player->m_PlayerPos.x -= 3.0f * XEngine->GetTween();
          Player->m_PlayerState = PS_RUNLEFT;

          if (Player->m_Direction > 0) {
            Player->m_FrameTime -= 0.25f * XEngine->GetTween();
          } else {
            Player->m_FrameTime += 0.25f * XEngine->GetTween();
          }
        } else {
          Player->m_PlayerState = PS_STAND;
          Player->m_FrameTime = 0.0f;
        }

        if (Player->m_FrameTime < 0.0f) {
          Player->m_FrameTime += 40.0f;
        }

        float Theta = ATan2(Player->m_AimerPos.x - Player->m_PlayerPos.x,
                            Player->m_AimerPos.y - Player->m_PlayerPos.y);

        Player->m_Rot = Theta;

        if (Theta < 0.0f) {
          Player->m_Direction = -1;

          Player->m_Weapon->m_HotPoint.x =
              Player->m_PlayerPos.x + Sin(Theta - 15.0f) * 32.0f;
          Player->m_Weapon->m_HotPoint.y =
              Player->m_PlayerPos.y + Cos(Theta - 15.0f) * 32.0f;

        } else {
          Player->m_Direction = 1;

          Player->m_Weapon->m_HotPoint.x =
              Player->m_PlayerPos.x + Sin(Theta + 15.0f) * 32.0f;
          Player->m_Weapon->m_HotPoint.y =
              Player->m_PlayerPos.y + Cos(Theta + 15.0f) * 32.0f;
        }

        if (Player->m_PlayerState == PS_STAND) {
          Player->m_CurrentFrame = 4;
        } else {
          Player->m_CurrentFrame = int(Player->m_FrameTime) % 4;
        }

        int NewStepSoundTime = int(Player->m_FrameTime / 3.0f) % 4;

        if (NewStepSoundTime != Player->m_StepSoundTime) {
          m_StepSound[rand() % 4].PlaySound();
          m_FloorSound[rand() % 4].PlaySound();

          Player->m_StepSoundTime = NewStepSoundTime;
        }

        if (Player->m_PlayerPos.x < 32.0f)
          Player->m_PlayerPos.x = 32.0f;
        if (Player->m_PlayerPos.x > 640.0f - 32.0f)
          Player->m_PlayerPos.x = 640.0f - 32.0f;

        if (Player->m_AimerPos.x < 32.0f)
          Player->m_AimerPos.x = 32.0f;
        if (Player->m_AimerPos.x > 640.0f - 32.0f)
          Player->m_AimerPos.x = 640.0f - 32.0f;
        if (Player->m_AimerPos.y < 32.0f)
          Player->m_AimerPos.y = 32.0f;
        if (Player->m_AimerPos.y > 440.0f)
          Player->m_AimerPos.y = 440.0f;

        for (std::list<CBaby *>::iterator iBaby = m_BabyList.begin(),
                                          iBaby_next = iBaby;
             iBaby != m_BabyList.end(); iBaby = iBaby_next) {
          iBaby_next = iBaby;
          ++iBaby_next;
          CBaby *Baby = (*iBaby);

          if (Baby->m_BabyState == BS_WALKLEFT ||
              Baby->m_BabyState == BS_WALKRIGHT) {
            float DX = Baby->m_Pos.x - Player->m_PlayerPos.x;
            float DY = Baby->m_Pos.y - Player->m_PlayerPos.y;

            float Dist = Sqrt(DX * DX + DY * DY);

            if (Dist < 24.0f) {
              Baby->m_Health = -1;
            }
          }
        }

        for (std::list<CWeaponBox *>::iterator iWeaponBox = m_BoxList.begin(),
                                               iWeaponBox_next = iWeaponBox;
             iWeaponBox != m_BoxList.end(); iWeaponBox = iWeaponBox_next) {
          iWeaponBox_next = iWeaponBox;
          ++iWeaponBox_next;
          CWeaponBox *WeaponBox = (*iWeaponBox);

          float DX = WeaponBox->m_Pos.x - Player->m_PlayerPos.x;
          float DY = WeaponBox->m_Pos.y - Player->m_PlayerPos.y;

          float Dist = Sqrt(DX * DX + DY * DY);

          if (Dist < 24.0f) {
            bool Collected = false;

            if (WeaponBox->m_Special) {
              switch (WeaponBox->m_WeaponType) {
              case ST_NUKE:

                if (XEngine->GamePad[Player->m_ControllerNum]
                        .bAnalogButtons[0] > 75) {
                  if (Player->m_BlackSpecial != NULL) {

                    Game.m_SpecialList.remove(Player->m_BlackSpecial);
                    delete Player->m_BlackSpecial;
                  }

                  Player->m_BlackSpecial = new CNuke{};
                  Player->m_BlackSpecial->m_Player = Player;
                  Game.m_SpecialList.push_back(Player->m_BlackSpecial);

                  Collected = true;
                }

                break;

              case ST_SHIELD:

                if (XEngine->GamePad[Player->m_ControllerNum]
                        .bAnalogButtons[0] > 75) {
                  if (Player->m_WhiteSpecial != NULL) {
                    Game.m_SpecialList.remove(Player->m_WhiteSpecial);
                    delete Player->m_WhiteSpecial;
                  }

                  Player->m_WhiteSpecial = new CShield{};
                  Player->m_WhiteSpecial->m_Player = Player;
                  Game.m_SpecialList.push_back(Player->m_WhiteSpecial);

                  Collected = true;
                }

                break;

              case ST_GRAVITY:

                if (XEngine->GamePad[Player->m_ControllerNum]
                        .bAnalogButtons[0] > 75) {
                  if (Player->m_WhiteSpecial != NULL) {
                    Game.m_SpecialList.remove(Player->m_WhiteSpecial);
                    delete Player->m_WhiteSpecial;
                  }

                  Player->m_WhiteSpecial = new CGravity{};
                  Player->m_WhiteSpecial->m_Player = Player;
                  Game.m_SpecialList.push_back(Player->m_WhiteSpecial);

                  Collected = true;
                }

                break;

              case ST_TIME:

                if (XEngine->GamePad[Player->m_ControllerNum]
                        .bAnalogButtons[0] > 75) {
                  if (Player->m_WhiteSpecial != NULL) {
                    Game.m_SpecialList.remove(Player->m_WhiteSpecial);
                    delete Player->m_WhiteSpecial;
                  }

                  Player->m_WhiteSpecial = new CTime{};
                  Player->m_WhiteSpecial->m_Player = Player;
                  Game.m_SpecialList.push_back(Player->m_WhiteSpecial);

                  Collected = true;
                }

                break;

              case ST_IONCANNON:

                if (XEngine->GamePad[Player->m_ControllerNum]
                        .bAnalogButtons[0] > 75) {
                  if (Player->m_BlackSpecial != NULL) {
                    Game.m_SpecialList.remove(Player->m_BlackSpecial);
                    delete Player->m_BlackSpecial;
                  }

                  Player->m_BlackSpecial = new CIonCannon{};
                  Player->m_BlackSpecial->m_Player = Player;
                  Game.m_SpecialList.push_back(Player->m_BlackSpecial);

                  Collected = true;
                }

                break;
              }
            } else {
              if (XEngine->GamePad[Player->m_ControllerNum].bAnalogButtons[0] >
                  75) {
                Collected = true;

                if (WeaponBox->m_WeaponType == WT_RANDOM) {
                  WeaponBox->m_WeaponType = rand() % (W_NUM_WEAPONS - 1) + 1;
                }

                if (Player->m_Weapon->m_WeaponType == WeaponBox->m_WeaponType) {
                  Player->m_Weapon->Upgrade();
                  fprintf(stderr, "WEAPON upgrade player=%d type=%d level=%d cap=%d\n",
                          Player->m_ControllerNum + 1, Player->m_Weapon->m_WeaponType,
                          Player->m_Weapon->m_PowerLevel, WeaponPowerLimit());

                  m_WeaponUpgradeSound.PlaySound();
                } else {

                  for (std::list<CRocket *>::iterator
                           iRocket = Game.m_RocketList.begin(),
                           iRocket_next = iRocket;
                       iRocket != Game.m_RocketList.end();
                       iRocket = iRocket_next) {
                    iRocket_next = iRocket;
                    ++iRocket_next;
                    CRocket *Rocket = (*iRocket);

                    if (Rocket->m_Launcher == Player->m_Weapon) {
                      Explode(Rocket->m_Pos.x, Rocket->m_Pos.y, Player);

                      Rocket->m_Launcher->m_NumRockets--;

                      std::list<CRocket *>::iterator iElement = iRocket;

                      Game.m_RocketList.erase(iElement);
                      delete Rocket;
                    }
                  }

                  Player->m_Weapon->Stop();

                  switch (WeaponBox->m_WeaponType) {
                  case WT_ROCKETS:

                    Player->m_Weapon = OwnWeapon<CRockets>();
                    break;

                  case WT_RAILGUN:

                    Player->m_Weapon = OwnWeapon<CRailgun>();
                    break;

                  case WT_MACHINEGUN:

                    Player->m_Weapon = OwnWeapon<CMachineGun>();
                    break;

                  case WT_PLASMA:

                    Player->m_Weapon = OwnWeapon<CPlasma>();
                    break;

                  case WT_GRENADES:

                    Player->m_Weapon = OwnWeapon<CGrenades>();
                    break;

                  case WT_FLAMETHROWER:

                    Player->m_Weapon = OwnWeapon<CFlameThrower>();
                    break;
                  }

                  Player->m_Weapon->m_Player = Player;
                }
              } else {
                Collected = false;
              }
            }

            if (Collected) {
              m_WeaponPickupSound.PlaySound();

              Spark *MySpark;

              for (float Theta = 0.0f; Theta < 360.0f; Theta += 20.0f) {
                MySpark = new Spark{};

                MySpark->m_Speed.x = Sin(Theta) * 1.0f;
                MySpark->m_Speed.y = Cos(Theta) * 1.0f;

                MySpark->m_Pos.x = WeaponBox->m_Pos.x;
                MySpark->m_Pos.y = WeaponBox->m_Pos.y;

                MySpark->m_Life = 30.0f;
                MySpark->m_Rot = Rand() * 360.0f;

                Game.m_SparkList.push_back(MySpark);
              }

              std::list<CWeaponBox *>::iterator iElement = iWeaponBox;

              m_BoxList.erase(iElement);
              delete WeaponBox;
            }
          }
        }

        Player->m_Weapon->m_Target.x = Player->m_AimerPos.x;
        Player->m_Weapon->m_Target.y = Player->m_AimerPos.y;

        if (XEngine->GamePad[Player->m_ControllerNum].bAnalogButtons[6] > 75) {
          Player->m_Weapon->FireAlternate();
        }

        if (XEngine->GamePad[Player->m_ControllerNum].bAnalogButtons[7] > 75) {
          Player->m_Weapon->Fire();
        }

        if (XEngine->GamePad[Player->m_ControllerNum].bAnalogButtons[4] > 75) {
          if (Player->m_BlackSpecial != NULL) {
            Player->m_BlackSpecial->m_Activated = true;
            Player->m_BlackSpecial->Activate();

            Player->m_BlackSpecial = NULL;
          }
        }

        if (XEngine->GamePad[Player->m_ControllerNum].bAnalogButtons[5] > 75) {
          if (Player->m_WhiteSpecial != NULL) {
            Player->m_WhiteSpecial->m_Activated = true;
            Player->m_WhiteSpecial->Activate();

            Player->m_WhiteSpecial = NULL;
          }
        }

        Player->m_Weapon->Update();
      }

      if (Player->m_Lives < 0) {
        if (Player->m_Score > m_BestScores[Player->m_ControllerNum]) {
          m_BestScores[Player->m_ControllerNum] = Player->m_Score;
        }

        XEngine->GamePad[Player->m_ControllerNum]
            .Feedback.Rumble.wLeftMotorSpeed = WORD(0);
        XEngine->GamePad[Player->m_ControllerNum]
            .Feedback.Rumble.wRightMotorSpeed = WORD(0);

        if (XEngine->GamePad[Player->m_ControllerNum].hDevice != 0x00000000)
          XInputSetState(XEngine->GamePad[Player->m_ControllerNum].hDevice,
                         &XEngine->GamePad[Player->m_ControllerNum].Feedback);

        for (std::list<CRocket *>::iterator iRocket = Game.m_RocketList.begin(),
                                            iRocket_next = iRocket;
             iRocket != Game.m_RocketList.end(); iRocket = iRocket_next) {
          iRocket_next = iRocket;
          ++iRocket_next;
          CRocket *Rocket = (*iRocket);

          if (Rocket->m_Launcher == Player->m_Weapon) {
            Explode(Rocket->m_Pos.x, Rocket->m_Pos.y, Player);

            Rocket->m_Launcher->m_NumRockets--;

            std::list<CRocket *>::iterator iElement = iRocket;

            Game.m_RocketList.erase(iElement);
            delete Rocket;
          }
        }

        // delete Player->m_Weapon;

        for (std::list<CBaby *>::iterator iBaby = m_BabyList.begin(),
                                          iBaby_next = iBaby;
             iBaby != m_BabyList.end(); iBaby = iBaby_next) {
          iBaby_next = iBaby;
          ++iBaby_next;
          CBaby *Baby = (*iBaby);

          if (Baby->m_LastHit == Player) {
            Baby->m_LastHit = NULL;
          }
        }

        m_PlayerList.remove(Player);
        // delete Player;

        m_NumPlayers--;
        m_Continues[Player->m_ControllerNum]--;
      }
    }

    for (std::list<CSpecial *>::iterator iSpecial = m_SpecialList.begin(),
                                         iSpecial_next = iSpecial;
         iSpecial != m_SpecialList.end(); iSpecial = iSpecial_next) {
      iSpecial_next = iSpecial;
      ++iSpecial_next;
      CSpecial *Special = (*iSpecial);

      if (Special->m_Activated) {
        Special->Update();

        Special->m_Life -= 1.0f * XEngine->GetTween();

        if (Special->m_Life <= 0.0f) {

          m_SpecialList.remove(Special);
          delete Special;
        }
      }
    }

    for (std::list<CBaby *>::iterator iBaby = m_BabyList.begin(),
                                      iBaby_next = iBaby;
         iBaby != m_BabyList.end(); iBaby = iBaby_next) {
      iBaby_next = iBaby;
      ++iBaby_next;
      CBaby *Baby = (*iBaby);

      switch (Baby->m_BabyState) {
      case BS_FLOATINGLEFT:

        Baby->m_Pos.x -= XEngine->GetTween() * (Baby->m_ThreatLevel + 1.0f) *
                         G_BABY_FLOAT_SPEED;

        Baby->m_Rot += 2.0f * XEngine->GetTween();
        Baby->m_Pos.y += Sin(Baby->m_Rot) / 2.0f * XEngine->GetTween();

        if (Baby->m_Pos.x < -34.0f) {
          if (m_NumPlayers > 0)
            Baby->m_ThreatLevel++;
          Baby->m_BabyState = BS_FLOATINGRIGHT;
          Baby->m_HasBomb = true;
        }

        break;

      case BS_FLOATINGRIGHT:

        Baby->m_Pos.x += XEngine->GetTween() * (Baby->m_ThreatLevel + 1.0f) *
                         G_BABY_FLOAT_SPEED;

        Baby->m_Rot += 2.0f * XEngine->GetTween();
        Baby->m_Pos.y += Sin(Baby->m_Rot) / 2.0f * XEngine->GetTween();

        if (Baby->m_Pos.x > 640 + 32.0f) {
          if (m_NumPlayers > 0)
            Baby->m_ThreatLevel++;
          Baby->m_BabyState = BS_FLOATINGLEFT;
          Baby->m_HasBomb = true;
        }

        break;

      case BS_BOUNCING:

        Baby->m_Speed.y += 0.1f * XEngine->GetTween() * m_Gravity;

        Baby->m_Pos.x += Baby->m_Speed.x * XEngine->GetTween();
        Baby->m_Pos.y += Baby->m_Speed.y * XEngine->GetTween();

        if (Baby->m_Pos.x < 32.0f) {
          Baby->m_Speed.x = -Baby->m_Speed.x * 0.5f;
          Baby->m_Pos.x = 32.0f;
        }

        if (Baby->m_Pos.x > 640.0f - 32.0f) {
          Baby->m_Speed.x = -Baby->m_Speed.x * 0.5f;
          Baby->m_Pos.x = 640.0f - 32.0f;
        }

        if (Baby->m_Pos.y > 440.0f) {
          Baby->m_Speed.y = -Baby->m_Speed.y * 0.8f;
          Baby->m_Pos.y = 440.0f;
          Baby->m_Speed.x *= 0.5f;
          Baby->m_DeltaRot *= 0.8f;

          // Baby->m_OnFire=false;
          Baby->m_Combo = -1;
        }

        Baby->m_Rot += Baby->m_DeltaRot * XEngine->GetTween();

        if (Baby->m_Speed.y < .015f && Baby->m_Speed.y > -.015f &&
            Baby->m_Pos.y > 439.25f) {
          Baby->m_BabyState = BS_GETTINGUP;
          Baby->m_Stand = 32.0f;
          Baby->m_WalkFrame = 0;
        }

        break;

      case BS_GETTINGUP:

        Baby->m_Speed.y = 0.0f;
        Baby->m_Speed.x = 0.0f;

        Baby->m_Pos.y = 440.0f;
        Baby->m_Rot = 0.0f;

        Baby->m_Stand -= 1.0f * XEngine->GetTween();
        Baby->m_WalkFrame = (int(Baby->m_Stand / 7.0f));

        if (Baby->m_WalkFrame > 4)
          Baby->m_WalkFrame = 0;

        if (Baby->m_Stand <= 0.0f) {
          if (Rand() <= .5f) {
            Baby->m_BabyState = BS_WALKLEFT;
            Baby->m_WalkFrame = 0;
            Baby->m_FrameTime = 0.0f;
          } else {
            Baby->m_BabyState = BS_WALKRIGHT;
            Baby->m_WalkFrame = 0;
            Baby->m_FrameTime = 0.0f;
          }
        }

        break;

      case BS_WALKLEFT:

        Baby->m_Pos.x -= 0.5f * XEngine->GetTween();

        Baby->m_FrameTime += 0.2f * XEngine->GetTween();
        Baby->m_WalkFrame = (int(Baby->m_FrameTime) % 4);

        if (Baby->m_WalkFrame > 5) {
          Baby->m_WalkFrame = 0;
          Baby->m_FrameTime = 0.0f;
        }

        if (Baby->m_Pos.x <= -10.0f) {
          Baby->m_BabyState = BS_WALKRIGHT;
        }

        break;

      case BS_WALKRIGHT:

        Baby->m_Pos.x += 0.5f * XEngine->GetTween();

        Baby->m_FrameTime += 0.2f * XEngine->GetTween();
        Baby->m_WalkFrame = (int(Baby->m_FrameTime) % 4);

        if (Baby->m_WalkFrame > 5) {
          Baby->m_WalkFrame = 0;
          Baby->m_FrameTime = 0.0f;
        }

        if (Baby->m_Pos.x >= 650.0f) {
          Baby->m_BabyState = BS_WALKLEFT;
        }

        break;
      }

      if (Baby->m_OnFire) {
        Baby->m_OnFire--;

        if (Baby->m_OnFire % 8 == 0) {
          if (Baby->m_Burner !=
              NULL) // other tests needed here to make sure m_burner is valid
          {
            Baby->Damage(0.0f, 0.0f, 1, Baby->m_Burner, 1);
          }
        }

        Smoke *MySmoke;

        for (int i = 0; i < 2; i++) {
          MySmoke = new Smoke{};

          MySmoke->m_RocketSmoke = false;

          MySmoke->m_Pos.x = Baby->m_Pos.x;
          MySmoke->m_Pos.y = Baby->m_Pos.y;
          MySmoke->m_Speed.x = Rand() - 0.5f;
          MySmoke->m_Speed.y = Rand() - 0.5f;

          MySmoke->m_Life = 50.0f;
          MySmoke->m_Rot = Rand() * 360.0f;

          Game.m_SmokeList.push_back(MySmoke);
        }

        for (int i = 0; i < 2; i++) {
          Fire *MyFire = new Fire{};

          MyFire->m_Pos.x = Baby->m_Pos.x + Rand() * 5.0f - 2.5f;
          MyFire->m_Pos.y = Baby->m_Pos.y + Rand() * 5.0f - 2.5f;

          MyFire->m_Speed.x = Rand() * 1.0f - 0.5f;
          MyFire->m_Speed.y = Rand() * 1.0f - 0.5f;

          MyFire->m_Life = 50.0f;
          MyFire->m_Rot = Rand() * 360.0f;

          MyFire->m_SmallFire = true;

          Game.m_FireList.push_back(MyFire);
        }
      }

      if (Baby->m_ThreatLevel > 3) {
        Baby->m_ThreatLevel = 3;
      }

      if (Baby->m_BabyState == BS_FLOATINGLEFT ||
          Baby->m_BabyState == BS_FLOATINGRIGHT) {
        if (Baby->m_ThreatLevel > 0) {
          for (std::list<CPlayer *>::iterator iPlayer = m_PlayerList.begin(),
                                              iPlayer_next = iPlayer;
               iPlayer != m_PlayerList.end(); iPlayer = iPlayer_next) {
            iPlayer_next = iPlayer;
            ++iPlayer_next;
            CPlayer *Player = (*iPlayer);

            float Dist = fAbs(Baby->m_Pos.x - Player->m_PlayerPos.x);

            if (Dist <= 40.0f && Baby->m_HasBomb == true) {
              Baby->m_HasBomb = false;

              CBomb *MyBomb;

              MyBomb = new CBomb{};

              MyBomb->m_Size = float(Baby->m_ThreatLevel);
              MyBomb->m_Pos.y = Baby->m_Pos.y + 24.0f;
              MyBomb->m_Pos.x = Baby->m_Pos.x;
              MyBomb->m_Speed.y = 0.0f;

              if (Baby->m_BabyState == BS_FLOATINGLEFT) {
                MyBomb->m_Speed.x = -G_BABY_FLOAT_SPEED;
              } else {
                MyBomb->m_Speed.x = +G_BABY_FLOAT_SPEED;
              }

              m_BombList.push_back(MyBomb);
            }
          }
        }
      }

      if (Baby->m_Pos.y < -8.0f && Baby->m_BabyState == BS_BOUNCING) {
        if (Baby->m_LastHit != NULL) {
          Baby->m_LastHit->AddScore(500, Baby->m_Pos.x, 80.0f);
        }

        std::list<CBaby *>::iterator iElement = iBaby;

        m_BabyList.erase(iElement);
        delete Baby;
      } else if (Baby->m_Health <= 0) {
        EnemyExplode(Baby->m_Pos.x, Baby->m_Pos.y, Baby->m_Killer);

        Gib *MyGib;

        for (int i = 0; i < 25; i++) {
          MyGib = new Gib{};

          MyGib->m_Speed.x = Rand() * 10.0f - 5.0f;
          MyGib->m_Speed.y = Rand() * 16.0f - 8.0f;

          MyGib->m_Pos.x = Baby->m_Pos.x;
          MyGib->m_Pos.y = Baby->m_Pos.y;

          MyGib->m_Life = 320.0f;
          MyGib->m_Rot = Rand() * 360.0f;

          MyGib->m_RotDelta = Rand() * 32.0f - 16.0f;

          MyGib->m_GibType = rand() % 6;

          Game.m_GibList.push_back(MyGib);
        }

        m_GibSound[3].PlaySound();

        if (Baby->m_LastHit != NULL) {
          Baby->m_LastHit->AddScore(250, Baby->m_Pos.x, 80.0f);
        }

        std::list<CBaby *>::iterator iElement = iBaby;

        m_BabyList.erase(iElement);
        delete Baby;
      }
    }

    for (std::list<CBomb *>::iterator iCBomb = m_BombList.begin(),
                                      iCBomb_next = iCBomb;
         iCBomb != m_BombList.end(); iCBomb = iCBomb_next) {
      iCBomb_next = iCBomb;
      ++iCBomb_next;
      CBomb *MyBomb = (*iCBomb);

      MyBomb->m_Pos.y += MyBomb->m_Speed.y * XEngine->GetTween();
      MyBomb->m_Pos.x += MyBomb->m_Speed.x * XEngine->GetTween();

      MyBomb->m_Speed.y += 0.05f * XEngine->GetTween() * m_Gravity;

      MyBomb->m_Rot = ATan2(MyBomb->m_Speed.x, MyBomb->m_Speed.y);

      if (MyBomb->m_Size > 3) {
        MyBomb->m_Size = 3;
      }

      if (MyBomb->m_Pos.y >= 440) {
        EnemyExplode(MyBomb->m_Pos.x, MyBomb->m_Pos.y, NULL);

        std::list<CBomb *>::iterator iElement = iCBomb;

        m_BombList.erase(iElement);
        delete MyBomb;
      }
    }

    for (std::list<Balloon *>::iterator iBalloon = m_BalloonList.begin(),
                                        iBalloon_next = iBalloon;
         iBalloon != m_BalloonList.end(); iBalloon = iBalloon_next) {
      iBalloon_next = iBalloon;
      ++iBalloon_next;
      Balloon *MyBalloon = (*iBalloon);

      if (MyBalloon->m_Pos.y < -10.0f) {
        std::list<Balloon *>::iterator iElement = iBalloon;

        m_BalloonList.erase(iElement);
        delete MyBalloon;
      } else {
        MyBalloon->m_Speed.y -= 0.025f * XEngine->GetTween();

        MyBalloon->m_Pos.x += MyBalloon->m_Speed.x * XEngine->GetTween();
        MyBalloon->m_Pos.y += MyBalloon->m_Speed.y * XEngine->GetTween();
      }
    }

    for (std::list<Slug *>::iterator iSlug = Game.m_SlugList.begin(),
                                     iSlug_next = iSlug;
         iSlug != Game.m_SlugList.end(); iSlug = iSlug_next) {
      iSlug_next = iSlug;
      ++iSlug_next;
      Slug *MySlug = (*iSlug);

      if (MySlug->m_Life <= 0.0f) {
        std::list<Slug *>::iterator iElement = iSlug;

        m_SlugList.erase(iElement);
        delete MySlug;
      } else {
        MySlug->m_Life -= 1.0f * XEngine->GetTween();
      }
    }

    for (std::list<Grenade *>::iterator iGrenade = Game.m_GrenadeList.begin(),
                                        iGrenade_next = iGrenade;
         iGrenade != Game.m_GrenadeList.end(); iGrenade = iGrenade_next) {
      iGrenade_next = iGrenade;
      ++iGrenade_next;
      Grenade *MyGrenade = (*iGrenade);

      MyGrenade->m_Life -= 1.0f * XEngine->GetTween();

      MyGrenade->m_Speed.y += 0.2f * XEngine->GetTween() * m_Gravity;
      MyGrenade->m_Rot += MyGrenade->m_RotSpeed * XEngine->GetTween();

      MyGrenade->m_Pos.x += MyGrenade->m_Speed.x * XEngine->GetTween();
      MyGrenade->m_Pos.y += MyGrenade->m_Speed.y * XEngine->GetTween();

      COLLISIONINFO MyCollisionInfo;

      for (std::list<CBaby *>::iterator iBaby = Game.m_BabyList.begin(),
                                        iBaby_next = iBaby;
           iBaby != Game.m_BabyList.end(); iBaby = iBaby_next) {
        iBaby_next = iBaby;
        ++iBaby_next;
        CBaby *Baby = (*iBaby);

        MyCollisionInfo = XEngine->Collide(
            MyGrenade->m_Pos.x, MyGrenade->m_Pos.y, 8.0f, MyGrenade->m_Speed.x,
            MyGrenade->m_Speed.y, Baby->m_Pos.x, Baby->m_Pos.y, 16.0f);

        if (MyCollisionInfo.Collided) {
          break;
        }
      }

      if (MyCollisionInfo.Collided == false) {
        for (std::list<CWeaponBox *>::iterator iBox = Game.m_BoxList.begin(),
                                               iBox_next = iBox;
             iBox != Game.m_BoxList.end(); iBox = iBox_next) {
          iBox_next = iBox;
          ++iBox_next;
          CWeaponBox *WeaponBox = (*iBox);

          if (!(WeaponBox->m_BoxState == WBS_FALLING)) {
            MyCollisionInfo =
                XEngine->Collide(MyGrenade->m_Pos.x, MyGrenade->m_Pos.y, 8.0f,
                                 MyGrenade->m_Speed.x, MyGrenade->m_Speed.y,
                                 WeaponBox->m_Pos.x, WeaponBox->m_Pos.y, 16.0f);

            if (WeaponBox->m_BoxState == WBS_FLOATING &&
                MyCollisionInfo.Collided) {
              WeaponBox->m_BoxState = WBS_FALLING;
              break;
            }
          }
        }
      }

      if (MyGrenade->m_Pos.y > 460.0f) {
        MyGrenade->m_Speed.y = -MyGrenade->m_Speed.y * .75f;
        MyGrenade->m_Pos.y = 460.0f;

        if (MyGrenade->m_Speed.y > -.55f) {
          MyGrenade->m_Speed.x = 0.0f;
          MyGrenade->m_RotSpeed = 0.0f;
        } else {
          m_GrenadeBounceSound[rand() % 2].PlaySound();
          MyGrenade->m_RotSpeed = Rand() * 32.0f - 16.0f;
        }
      }

      if (MyGrenade->m_Life <= 0.0f || MyCollisionInfo.Collided == true) {
        Explode(MyGrenade->m_Pos.x, MyGrenade->m_Pos.y,
                MyGrenade->m_Launcher->m_Player);

        int Shrapnel = GrenadeFragments(MyGrenade->m_PowerLevel, false);

        for (int NumberofBullets = 0; NumberofBullets < Shrapnel;
             NumberofBullets++) {
          Bullet *MyBullet = new Bullet{};

          MyBullet->m_Player = MyGrenade->m_Player;

          MyBullet->m_Life = 30.0f;

          MyBullet->m_PowerLevel = 1;

          MyBullet->m_Pos.x = MyGrenade->m_Pos.x;
          MyBullet->m_Pos.y = MyGrenade->m_Pos.y;
          MyBullet->m_sPos.x = MyGrenade->m_Pos.x;
          MyBullet->m_sPos.y = MyGrenade->m_Pos.y;

          float Theta = Rand() * 360.0f;

          MyBullet->m_Speed.x = Sin(Theta) * 32.0f + Rand() * 10.0f;
          MyBullet->m_Speed.y = Cos(Theta) * 32.0f + Rand() * 10.0f;

          Game.m_BulletList.push_back(MyBullet);
        }

        std::list<Grenade *>::iterator iElement = iGrenade;

        m_GrenadeList.erase(iElement);
        delete MyGrenade;
      }
    }

    for (std::list<CRocket *>::iterator iRocket = Game.m_RocketList.begin(),
                                        iRocket_next = iRocket;
         iRocket != Game.m_RocketList.end(); iRocket = iRocket_next) {
      iRocket_next = iRocket;
      ++iRocket_next;
      CRocket *Rocket = (*iRocket);

      float MakeSmoke;

      if (Rocket->m_Fuel <= 0.0f) {
        Rocket->m_Speed.y += 0.15f * m_Gravity;

        Rocket->m_Rot -= 8.0f * XEngine->GetTween();
        MakeSmoke = Rand();
      } else {
        if (Rocket->m_Launcher->m_DumbFire == false) {
          Rocket->m_Rot =
              ATan2(Rocket->m_Launcher->m_Target.x - Rocket->m_Pos.x,
                    Rocket->m_Launcher->m_Target.y - Rocket->m_Pos.y);
        }

        MakeSmoke = 1.0f;
      }

      if (MakeSmoke > 0.6f) {
        Rocket->m_Speed.x +=
            Sin(Rocket->m_Rot) * W_ROCKET_ACCELERATION * XEngine->GetTween();
        Rocket->m_Speed.y +=
            Cos(Rocket->m_Rot) * W_ROCKET_ACCELERATION * XEngine->GetTween();

        Rocket->m_Fuel -= 1.0f * XEngine->GetTween();

        Smoke *MySmoke;

        MySmoke = new Smoke{};

        MySmoke->m_RocketSmoke = true;

        MySmoke->m_Pos.x = Rocket->m_Pos.x;
        MySmoke->m_Pos.y = Rocket->m_Pos.y;
        MySmoke->m_Speed.x = Rand() - 0.5f;
        MySmoke->m_Speed.y = Rand() - 0.5f;

        MySmoke->m_Life = 50.0f;
        MySmoke->m_Rot = Rand() * 360.0f;

        Game.m_SmokeList.push_back(MySmoke);
      }

      Rocket->m_Speed.x *= Pow(W_ROCKET_DECELERATION, XEngine->GetTween());
      Rocket->m_Speed.y *= Pow(W_ROCKET_DECELERATION, XEngine->GetTween());

      COLLISIONINFO MyCollisionInfo;

      for (std::list<CBaby *>::iterator iBaby = Game.m_BabyList.begin(),
                                        iBaby_next = iBaby;
           iBaby != Game.m_BabyList.end(); iBaby = iBaby_next) {
        iBaby_next = iBaby;
        ++iBaby_next;
        CBaby *Baby = (*iBaby);

        MyCollisionInfo = XEngine->Collide(
            Rocket->m_Pos.x, Rocket->m_Pos.y, 4.0f, Rocket->m_Speed.x,
            Rocket->m_Speed.y, Baby->m_Pos.x, Baby->m_Pos.y, 16.0f);

        if (MyCollisionInfo.Collided) {
          break;
        }
      }

      if (MyCollisionInfo.Collided == false) {
        for (std::list<CWeaponBox *>::iterator iBox = Game.m_BoxList.begin(),
                                               iBox_next = iBox;
             iBox != Game.m_BoxList.end(); iBox = iBox_next) {
          iBox_next = iBox;
          ++iBox_next;
          CWeaponBox *WeaponBox = (*iBox);

          if (!(WeaponBox->m_BoxState == WBS_FALLING)) {
            MyCollisionInfo =
                XEngine->Collide(Rocket->m_Pos.x, Rocket->m_Pos.y, 4.0f,
                                 Rocket->m_Speed.x, Rocket->m_Speed.y,
                                 WeaponBox->m_Pos.x, WeaponBox->m_Pos.y, 16.0f);

            if (WeaponBox->m_BoxState == WBS_FLOATING &&
                MyCollisionInfo.Collided) {
              WeaponBox->m_BoxState = WBS_FALLING;
              break;
            }
          }
        }
      }

      Rocket->m_Pos.x += Rocket->m_Speed.x * XEngine->GetTween();
      Rocket->m_Pos.y += Rocket->m_Speed.y * XEngine->GetTween();

      if ((Rocket->m_Pos.y <= -20 || Rocket->m_Pos.x <= -20 ||
           Rocket->m_Pos.x >= 660) &&
          Rocket->m_Launcher->m_DumbFire < 2) {
        Rocket->m_Launcher->m_NumRockets--;

        std::list<CRocket *>::iterator iElement = iRocket;

        m_RocketList.erase(iElement);
        delete Rocket;
      } else if (Rocket->m_Pos.y >= 460 || Rocket->m_Fuel < -100.0f ||
                 MyCollisionInfo.Collided == true) {
        Explode(Rocket->m_Pos.x, Rocket->m_Pos.y, Rocket->m_Launcher->m_Player);
        // Explode(Rocket->m_Pos.x,Rocket->m_Pos.y,NULL);
        // CBaby *Baby=new CBaby{};
        Rocket->m_Launcher->m_NumRockets--;

        std::list<CRocket *>::iterator iElement = iRocket;

        m_RocketList.erase(iElement);
        delete Rocket;
      }
    }

    for (std::list<Smoke *>::iterator iSmoke = Game.m_SmokeList.begin(),
                                      iSmoke_next = iSmoke;
         iSmoke != Game.m_SmokeList.end(); iSmoke = iSmoke_next) {
      iSmoke_next = iSmoke;
      ++iSmoke_next;
      Smoke *MySmoke = (*iSmoke);

      if (MySmoke->m_Life <= 0.0f) {
        std::list<Smoke *>::iterator iElement = iSmoke;

        m_SmokeList.erase(iElement); // WHAT THE FUCK IS WRONG WITH THIS FUCKING
                                     // LINE?????????????
        delete MySmoke;
      } else {
        MySmoke->m_Pos.x += MySmoke->m_Speed.x * XEngine->GetTween();
        MySmoke->m_Pos.y += MySmoke->m_Speed.y * XEngine->GetTween();

        MySmoke->m_Life -= 1.0f * XEngine->GetTween();
        MySmoke->m_Rot += 2.0f * XEngine->GetTween();
      }
    }

    for (std::list<Fire *>::iterator iFire = Game.m_FireList.begin(),
                                     iFire_next = iFire;
         iFire != Game.m_FireList.end(); iFire = iFire_next) {
      iFire_next = iFire;
      ++iFire_next;
      Fire *MyFire = (*iFire);

      if (MyFire->m_Life <= 0.0f) {
        std::list<Fire *>::iterator iElement = iFire;

        m_FireList.erase(iElement);
        delete MyFire;
      } else {
        MyFire->m_Pos.x += MyFire->m_Speed.x * XEngine->GetTween();
        MyFire->m_Pos.y += MyFire->m_Speed.y * XEngine->GetTween();

        MyFire->m_Life -= 1.0f * XEngine->GetTween();
        MyFire->m_Rot += 2.0f * XEngine->GetTween();
      }
    }

    for (std::list<Flame *>::iterator iFlame = Game.m_FlameList.begin(),
                                      iFlame_next = iFlame;
         iFlame != Game.m_FlameList.end(); iFlame = iFlame_next) {
      iFlame_next = iFlame;
      ++iFlame_next;
      Flame *MyFlame = (*iFlame);

      if (MyFlame->m_Life <= 0.0f) {
        std::list<Flame *>::iterator iElement = iFlame;

        m_FlameList.erase(iElement);
        delete MyFlame;
      } else {
        MyFlame->m_Pos.x += MyFlame->m_Speed.x * XEngine->GetTween();
        MyFlame->m_Pos.y += MyFlame->m_Speed.y * XEngine->GetTween();
        MyFlame->m_Rot += MyFlame->m_RotSpeed * XEngine->GetTween();

        MyFlame->m_Life -= 1.0f * XEngine->GetTween();

        for (std::list<CBaby *>::iterator iBaby = Game.m_BabyList.begin(),
                                          iBaby_next = iBaby;
             iBaby != Game.m_BabyList.end(); iBaby = iBaby_next) {
          iBaby_next = iBaby;
          ++iBaby_next;
          CBaby *Baby = (*iBaby);

          float DX = Baby->m_Pos.x - MyFlame->m_Pos.x;
          float DY = Baby->m_Pos.y - MyFlame->m_Pos.y;
          float DL = Sqrt(DX * DX + DY * DY);

          if (DL * 2.0f < 100.0f - MyFlame->m_Life * 3.0f +
                              MyFlame->m_PowerLevel * 25 &&
              Baby->m_Hit <= 0) {
            Baby->Damage(MyFlame->m_Speed.x / 8.0f, MyFlame->m_Speed.y / 8.0f,
                         1, MyFlame->m_Player, 1);
            Baby->m_OnFire = 100;
            Baby->m_Burner = MyFlame->m_Player;

            m_FlameThrowerHitSound.PlaySound();
          }
        }

        for (std::list<CWeaponBox *>::iterator iBox = Game.m_BoxList.begin(),
                                               iBox_next = iBox;
             iBox != Game.m_BoxList.end(); iBox = iBox_next) {
          iBox_next = iBox;
          ++iBox_next;
          CWeaponBox *WeaponBox = (*iBox);

          if (!(WeaponBox->m_BoxState == WBS_FALLING)) {
            float DX = WeaponBox->m_Pos.x - MyFlame->m_Pos.x;
            float DY = WeaponBox->m_Pos.y - MyFlame->m_Pos.y;
            float DL = Sqrt(DX * DX + DY * DY);

            if (WeaponBox->m_BoxState == WBS_FLOATING &&
                (DL * 2.0f < 100.0f - MyFlame->m_Life * 3.0f +
                                 MyFlame->m_PowerLevel * 25)) {
              WeaponBox->m_BoxState = WBS_FALLING;
              break;
            }
          }
        }
      }
    }

    for (std::list<Spark *>::iterator iSpark = Game.m_SparkList.begin(),
                                      iSpark_next = iSpark;
         iSpark != Game.m_SparkList.end(); iSpark = iSpark_next) {
      iSpark_next = iSpark;
      ++iSpark_next;
      Spark *MySpark = (*iSpark);

      if (MySpark->m_Life <= 0.0f) {
        std::list<Spark *>::iterator iElement = iSpark;

        m_SparkList.erase(iElement);
        delete MySpark;
      } else {
        MySpark->m_Speed.y += 0.01f * XEngine->GetTween() * m_Gravity;

        MySpark->m_Pos.x += MySpark->m_Speed.x * XEngine->GetTween();
        MySpark->m_Pos.y += MySpark->m_Speed.y * XEngine->GetTween();

        MySpark->m_Life -= 1.0f * XEngine->GetTween();
        MySpark->m_Rot += 2.0f * XEngine->GetTween();
      }
    }

    for (std::list<Corona *>::iterator iCorona = Game.m_CoronaList.begin(),
                                       iCorona_next = iCorona;
         iCorona != Game.m_CoronaList.end(); iCorona = iCorona_next) {
      iCorona_next = iCorona;
      ++iCorona_next;
      Corona *MyCorona = (*iCorona);

      if (MyCorona->m_Life <= 0.0f) {
        std::list<Corona *>::iterator iElement = iCorona;

        m_CoronaList.erase(iElement);
        delete MyCorona;
      } else {
        MyCorona->m_Life -= 1.0f * XEngine->GetTween();
        // MySpark->m_Rot+=2.0f*XEngine->GetTween();
      }
    }

    for (std::list<Gib *>::iterator iGib = Game.m_GibList.begin(),
                                    iGib_next = iGib;
         iGib != Game.m_GibList.end(); iGib = iGib_next) {
      iGib_next = iGib;
      ++iGib_next;
      Gib *MyGib = (*iGib);

      if (MyGib->m_Life <= 0.0f) {
        std::list<Gib *>::iterator iElement = iGib;

        m_GibList.erase(iElement);
        delete MyGib;
      } else {
        MyGib->m_Speed.y += 0.1f * XEngine->GetTween() * m_Gravity;

        MyGib->m_Pos.x += MyGib->m_Speed.x * XEngine->GetTween();
        MyGib->m_Pos.y += MyGib->m_Speed.y * XEngine->GetTween();

        MyGib->m_Life -= 1.0f * XEngine->GetTween();
        MyGib->m_Rot += MyGib->m_RotDelta * XEngine->GetTween();

        if (MyGib->m_Pos.x < 0.0f) {
          MyGib->m_Pos.x = 0.0f;
          MyGib->m_Speed.x = -MyGib->m_Speed.x;

          m_GibSound[rand() % 3].PlaySound();
        }

        if (MyGib->m_Pos.x > 640.0f) {
          MyGib->m_Pos.x = 640.0f;
          MyGib->m_Speed.x = -MyGib->m_Speed.x;

          m_GibSound[rand() % 3].PlaySound();
        }

        if (MyGib->m_Pos.y > 460.0f) {
          MyGib->m_RotDelta *= -0.8f;

          if (MyGib->m_Pos.y > 460.5f && MyGib->m_Pos.y < 465.0f &&
              rand() % 2 == 0)
            m_GibSound[rand() % 3].PlaySound();

          MyGib->m_Pos.y = 460.0f;

          MyGib->m_Speed.x *= 0.5f;
          MyGib->m_Speed.y = -MyGib->m_Speed.y * 0.25f;
        }
      }
    }

    for (std::list<BulletSpark *>::iterator
             iBulletSpark = Game.m_BulletSparkList.begin(),
             iBulletSpark_next = iBulletSpark;
         iBulletSpark != Game.m_BulletSparkList.end();
         iBulletSpark = iBulletSpark_next) {
      iBulletSpark_next = iBulletSpark;
      ++iBulletSpark_next;
      BulletSpark *MyBulletSpark = (*iBulletSpark);

      if (MyBulletSpark->m_Life <= 0.0f) {
        std::list<BulletSpark *>::iterator iElement = iBulletSpark;

        m_BulletSparkList.erase(iElement);
        delete MyBulletSpark;
      } else {
        MyBulletSpark->m_Speed.y += 0.1f * XEngine->GetTween() * m_Gravity;

        MyBulletSpark->m_Pos.x +=
            MyBulletSpark->m_Speed.x * XEngine->GetTween();
        MyBulletSpark->m_Pos.y +=
            MyBulletSpark->m_Speed.y * XEngine->GetTween();

        MyBulletSpark->m_Life -= 1.0f * XEngine->GetTween();
      }
    }

    for (std::list<ScreenShake *>::iterator
             iScreenShake = Game.m_ScreenShakeList.begin(),
             iScreenShake_next = iScreenShake;
         iScreenShake != Game.m_ScreenShakeList.end();
         iScreenShake = iScreenShake_next) {
      iScreenShake_next = iScreenShake;
      ++iScreenShake_next;
      ScreenShake *MyScreenShake = (*iScreenShake);

      if (MyScreenShake->m_Life <= 0.0f) {
        std::list<ScreenShake *>::iterator iElement = iScreenShake;

        m_ScreenShakeList.erase(iElement);
        delete MyScreenShake;
      } else {
        MyScreenShake->m_Life -= 1.0f * XEngine->GetTween();
        if (MyScreenShake->m_Life < 0.0f)
          MyScreenShake->m_Life = 0.0f;
      }
    }

    for (std::list<Bullet *>::iterator iBullet = Game.m_BulletList.begin(),
                                       iBullet_next = iBullet;
         iBullet != Game.m_BulletList.end(); iBullet = iBullet_next) {
      iBullet_next = iBullet;
      ++iBullet_next;
      Bullet *MyBullet = (*iBullet);

      if (MyBullet->m_Life <= 0.0f) {
        std::list<Bullet *>::iterator iElement = iBullet;

        m_BulletList.erase(iElement);
        delete MyBullet;
      } else {
        float Delta = (30.0f - MyBullet->m_Life) - 4.0f;
        if (Delta < 0.0f)
          Delta = 0.0f;

        COLLISIONINFO MyCollisionInfo;

        CBaby *HitBaby = NULL;
        CWeaponBox *HitBox = NULL;

        float Closest = 1000000.0f;

        /*
        if (MyBullet->m_Pos.y >= 460.0f)
        {
                Closest = 999999.0f;
        }
        */

        for (std::list<CWeaponBox *>::iterator iBox = Game.m_BoxList.begin(),
                                               iBox_next = iBox;
             iBox != Game.m_BoxList.end(); iBox = iBox_next) {
          iBox_next = iBox;
          ++iBox_next;
          CWeaponBox *WeaponBox = (*iBox);

          MyCollisionInfo =
              XEngine->Collide(MyBullet->m_Pos.x, MyBullet->m_Pos.y, 2.0f,
                               MyBullet->m_Speed.x * XEngine->GetTween(),
                               MyBullet->m_Speed.y * XEngine->GetTween(),
                               WeaponBox->m_Pos.x, WeaponBox->m_Pos.y, 16.0f);

          if (MyCollisionInfo.Collided &&
              WeaponBox->m_BoxState == WBS_FLOATING) {
            float DX = WeaponBox->m_Pos.x - MyBullet->m_Pos.x;
            float DY = WeaponBox->m_Pos.y - MyBullet->m_Pos.y;
            float Dist = DX * DX + DY * DY;

            if (Dist < Closest) {
              Closest = Dist;
              HitBox = WeaponBox;
            }
          }
        }

        for (std::list<CBaby *>::iterator iBaby = Game.m_BabyList.begin(),
                                          iBaby_next = iBaby;
             iBaby != Game.m_BabyList.end(); iBaby = iBaby_next) {
          iBaby_next = iBaby;
          ++iBaby_next;
          CBaby *Baby = (*iBaby);
          MyCollisionInfo =
              XEngine->Collide(MyBullet->m_Pos.x, MyBullet->m_Pos.y, 2.0f,
                               MyBullet->m_Speed.x * XEngine->GetTween(),
                               MyBullet->m_Speed.y * XEngine->GetTween(),
                               Baby->m_Pos.x, Baby->m_Pos.y, 16.0f);

          if (MyCollisionInfo.Collided) {
            float DX = Baby->m_Pos.x - MyBullet->m_Pos.x;
            float DY = Baby->m_Pos.y - MyBullet->m_Pos.y;

            float Dist = DX * DX + DY * DY;

            if (Dist < Closest) {
              Closest = Dist;
              HitBaby = Baby;
            }
          }
        }

        if (Closest < 1000000.0f) {
          float IX = 0.0f, IY = 0.0f;

          if (HitBaby != NULL) {
            if (MyBullet->m_PowerLevel >= 1 && MyBullet->m_PowerLevel <= 5) {
              float divisor = BulletKnockbackDivisor(MyBullet->m_PowerLevel);
              HitBaby->Damage(MyBullet->m_Speed.x / divisor,
                              MyBullet->m_Speed.y / divisor,
                              MyBullet->m_PowerLevel, MyBullet->m_Player, 0);
            }

            IX = HitBaby->m_Pos.x;
            IY = HitBaby->m_Pos.y;
          }

          if (HitBox != NULL) {
            HitBox->m_BoxState = WBS_FALLING;

            IX = HitBox->m_Pos.x;
            IY = HitBox->m_Pos.y;
          }

          BulletSpark *MyBulletSpark;

          for (int i = 0; i < 4; i++) {
            MyBulletSpark = new BulletSpark{};

            MyBulletSpark->m_Pos.x = IX + Rand() * 32.0f - 16.0f;
            MyBulletSpark->m_Pos.y = IY + Rand() * 32.0f - 16.0f;

            MyBulletSpark->m_Speed.x = Rand() * 16.0f - 8.0f;
            MyBulletSpark->m_Speed.y = Rand() * 16.0f - 8.0f;

            MyBulletSpark->m_Life = 10.0f;

            Game.m_BulletSparkList.push_back(MyBulletSpark);
          }

          std::list<Bullet *>::iterator iElement = iBullet;

          m_BulletList.erase(iElement);
          delete MyBullet;

          m_BulletHitSound[rand() % 3].PlaySound();

        } else {
          MyBullet->m_oPos.x =
              MyBullet->m_sPos.x + MyBullet->m_Speed.x * Delta * 30.0f;
          MyBullet->m_oPos.y =
              MyBullet->m_sPos.y + MyBullet->m_Speed.y * Delta * 30.0f;

          MyBullet->m_Pos.x += MyBullet->m_Speed.x * XEngine->GetTween();
          MyBullet->m_Pos.y += MyBullet->m_Speed.y * XEngine->GetTween();

          MyBullet->m_Life -= 1.0f * XEngine->GetTween();
        }
      }
    }

    for (std::list<Plasma *>::iterator iPlasma = Game.m_PlasmaList.begin(),
                                       iPlasma_next = iPlasma;
         iPlasma != Game.m_PlasmaList.end(); iPlasma = iPlasma_next) {
      iPlasma_next = iPlasma;
      ++iPlasma_next;
      Plasma *MyPlasma = (*iPlasma);

      if (MyPlasma->m_Life <= 0.0f) {
        std::list<Plasma *>::iterator iElement = iPlasma;

        m_PlasmaList.erase(iElement);
        delete MyPlasma;
      } else {
        COLLISIONINFO MyCollisionInfo;

        CBaby *HitBaby = NULL;
        CWeaponBox *HitBox = NULL;

        float Closest = 1000000.0f;

        // destroy plasma bolt if it hits the ground

        /*
        if (MyPlasma->m_Pos.y >= 460.0f)
        {
                Closest = 999999.0f;
        }
        */

        for (std::list<CWeaponBox *>::iterator iBox = Game.m_BoxList.begin(),
                                               iBox_next = iBox;
             iBox != Game.m_BoxList.end(); iBox = iBox_next) {
          iBox_next = iBox;
          ++iBox_next;
          CWeaponBox *WeaponBox = (*iBox);

          MyCollisionInfo =
              XEngine->Collide(MyPlasma->m_Pos.x, MyPlasma->m_Pos.y, 8.0f,
                               MyPlasma->m_Speed.x * XEngine->GetTween(),
                               MyPlasma->m_Speed.y * XEngine->GetTween(),
                               WeaponBox->m_Pos.x, WeaponBox->m_Pos.y, 16.0f);

          if (MyCollisionInfo.Collided &&
              WeaponBox->m_BoxState == WBS_FLOATING) {
            float DX = WeaponBox->m_Pos.x - MyPlasma->m_Pos.x;
            float DY = WeaponBox->m_Pos.y - MyPlasma->m_Pos.y;

            float Dist = DX * DX + DY * DY;

            if (Dist < Closest) {
              Closest = Dist;
              HitBox = WeaponBox;
            }
          }
        }

        for (std::list<CBaby *>::iterator iBaby = Game.m_BabyList.begin(),
                                          iBaby_next = iBaby;
             iBaby != Game.m_BabyList.end(); iBaby = iBaby_next) {
          iBaby_next = iBaby;
          ++iBaby_next;
          CBaby *Baby = (*iBaby);
          MyCollisionInfo =
              XEngine->Collide(MyPlasma->m_Pos.x, MyPlasma->m_Pos.y, 8.0f,
                               MyPlasma->m_Speed.x * XEngine->GetTween(),
                               MyPlasma->m_Speed.y * XEngine->GetTween(),
                               Baby->m_Pos.x, Baby->m_Pos.y, 16.0f);

          if (MyCollisionInfo.Collided) {
            float DX = Baby->m_Pos.x - MyPlasma->m_Pos.x;
            float DY = Baby->m_Pos.y - MyPlasma->m_Pos.y;

            float Dist = DX * DX + DY * DY;

            if (Dist < Closest) {
              Closest = Dist;
              HitBaby = Baby;
            }
          }
        }

        if (Closest < 1000000.0f) {
          float IX = 0.0f, IY = 0.0f;

          if (HitBaby != NULL) {
            switch (MyPlasma->m_PowerLevel) {
            case 1:

              HitBaby->Damage(
                  MyPlasma->m_Speed.x / 5.0f, MyPlasma->m_Speed.y / 5.0f,
                  MyPlasma->m_PowerLevel + 3, MyPlasma->m_Player, 0);
              break;

            case 2:

              HitBaby->Damage(
                  MyPlasma->m_Speed.x / 5.0f, MyPlasma->m_Speed.y / 5.0f,
                  MyPlasma->m_PowerLevel + 3, MyPlasma->m_Player, 0);
              break;

            case 3:

              HitBaby->Damage(
                  MyPlasma->m_Speed.x / 5.0f, MyPlasma->m_Speed.y / 5.0f,
                  MyPlasma->m_PowerLevel + 3, MyPlasma->m_Player, 0);
              break;
            }

            IX = HitBaby->m_Pos.x;
            IY = HitBaby->m_Pos.y;
          }

          if (HitBox != NULL) {
            HitBox->m_BoxState = WBS_FALLING;

            IX = HitBox->m_Pos.x;
            IY = HitBox->m_Pos.y;
          }

          Spark *MySpark;

          for (int i = 0; i < 8; i++) {
            MySpark = new Spark{};

            MySpark->m_Pos.x = IX + Rand() * 32.0f - 16.0f;
            MySpark->m_Pos.y = IY + Rand() * 32.0f - 16.0f;

            MySpark->m_Speed.x = Rand() * 16.0f - 8.0f;
            MySpark->m_Speed.y = Rand() * 16.0f - 8.0f;

            MySpark->m_Life = 30.0f;

            Game.m_SparkList.push_back(MySpark);
          }

          std::list<Plasma *>::iterator iElement = iPlasma;

          m_PlasmaList.erase(iElement);
          delete MyPlasma;

          m_PlasmaHitSound.PlaySound();
        } else {
          MyPlasma->m_Pos.x += MyPlasma->m_Speed.x * XEngine->GetTween();
          MyPlasma->m_Pos.y += MyPlasma->m_Speed.y * XEngine->GetTween();

          MyPlasma->m_Rot = ATan2(MyPlasma->m_Speed.x, MyPlasma->m_Speed.y);

          MyPlasma->m_Life -= 1.0f * XEngine->GetTween();
        }
      }
    }

    for (std::list<Score *>::iterator iScore = m_ScoreList.begin(),
                                      iScore_next = iScore;
         iScore != m_ScoreList.end(); iScore = iScore_next) {
      iScore_next = iScore;
      ++iScore_next;
      Score *MyScore = (*iScore);

      if (MyScore->m_Life <= 0.0f) {
        std::list<Score *>::iterator iElement = iScore;

        m_ScoreList.erase(iElement);
        delete MyScore;
      } else {
        MyScore->m_Pos.y -= 0.5f * XEngine->GetTween();
        MyScore->m_Life -= 1.0f * XEngine->GetTween();
      }
    }
  }

  /*
  XEngine->EnableMotionBlur();
  Refresh();//XEngine->RenderMotionBlur(0.9f);
  XEngine->DisableMotionBlur();
  */

  Refresh(); // XEngine->RenderMotionBlur(0.9f);
  XEngine->XUpdate();

  return 1;
}

float Retard = 0.0f;

void CGame::Refresh() {
#ifndef __ANDROID__
 if(NativeIsHeadlessInputTest())return;
#endif
  // screenshake
  float CameraPosX = 0.0f, CameraPosY = 0.0f;

  for (std::list<ScreenShake *>::iterator
           iScreenShake = Game.m_ScreenShakeList.begin(),
           iScreenShake_next = iScreenShake;
       iScreenShake != Game.m_ScreenShakeList.end();
       iScreenShake = iScreenShake_next) {
    iScreenShake_next = iScreenShake;
    ++iScreenShake_next;
    ScreenShake *MyScreenShake = (*iScreenShake);

    CameraPosX += Sin(MyScreenShake->m_Life * MyScreenShake->m_Speed) *
                  MyScreenShake->m_Amplitude * (MyScreenShake->m_Life / 30.0f);
    CameraPosY += Cos(MyScreenShake->m_Life * MyScreenShake->m_Speed) *
                  MyScreenShake->m_Amplitude * (MyScreenShake->m_Life / 30.0f);
  }

  float RumbleIntensity =
      Sqrt(CameraPosX * CameraPosX + CameraPosY * CameraPosY);

  for (std::list<CPlayer *>::iterator iPlayer = m_PlayerList.begin(),
                                      iPlayer_next = iPlayer;
       iPlayer != m_PlayerList.end(); iPlayer = iPlayer_next) {
    iPlayer_next = iPlayer;
    ++iPlayer_next;
    CPlayer *Player = (*iPlayer);

    XEngine->GamePad[Player->m_ControllerNum].Feedback.Rumble.wLeftMotorSpeed =
        WORD(RumbleIntensity * 65535.0f);
    XEngine->GamePad[Player->m_ControllerNum].Feedback.Rumble.wRightMotorSpeed =
        WORD(RumbleIntensity * 65535.0f);

    if (XEngine->GamePad[Player->m_ControllerNum].hDevice != 0x00000000)
      XInputSetState(XEngine->GamePad[Player->m_ControllerNum].hDevice,
                     &XEngine->GamePad[Player->m_ControllerNum].Feedback);
  }

  XEngine->SetCameraPos(CameraPosX, CameraPosY);

  // start drawing some game stuff
  XEngine->SetSpriteMode(SM_ALPHA_BLEND);
  XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);

  int FGScroll = int(m_Scroll * 2) % 640;
  int BG2Scroll = int(m_Scroll / 2.0f) % 640;
  int BG3Width = int(640.f * m_BGImage[3]->width / m_BGImage[3]->height);
  int BG3Scroll = int(m_Scroll / 4.0f) % BG3Width;

  XEngine->DrawSprite(m_BGImage[3], float(BG3Scroll), 240, float(BG3Width), 640.0f, 0);
  XEngine->DrawSprite(m_BGImage[3], float(BG3Scroll + BG3Width), 240, float(BG3Width), 640.0f,
                      0);
  XEngine->DrawSprite(m_BGImage[3], float(BG3Scroll + BG3Width * 2), 240, float(BG3Width), 640.0f,
                      0);

  XEngine->DrawSprite(m_BGImage[2], float(BG2Scroll), 240, 640, 640.0f, 0);
  XEngine->DrawSprite(m_BGImage[2], float(BG2Scroll + 640), 240, 640, 640.0f,
                      0);
  XEngine->DrawSprite(m_BGImage[2], float(BG2Scroll + 1280), 240, 640, 640.0f,
                      0);

  XEngine->DrawSprite(m_BGImage[1], 320.0f, 240, 640, 640.0f, 0);

  for (std::list<Balloon *>::iterator iBalloon = m_BalloonList.begin(),
                                      iBalloon_next = iBalloon;
       iBalloon != m_BalloonList.end(); iBalloon = iBalloon_next) {
    iBalloon_next = iBalloon;
    ++iBalloon_next;
    Balloon *MyBalloon = (*iBalloon);

    XEngine->DrawSprite(m_BalloonImage, MyBalloon->m_Pos.x, MyBalloon->m_Pos.y,
                        32, 32, 0.0f);
  }

  for (std::list<CBaby *>::iterator iBaby = m_BabyList.begin(),
                                    iBaby_next = iBaby;
       iBaby != m_BabyList.end(); iBaby = iBaby_next) {
    iBaby_next = iBaby;
    ++iBaby_next;
    CBaby *Baby = (*iBaby);

    switch (Baby->m_BabyState) {
    case BS_FLOATINGLEFT:

      XEngine->DrawSprite(m_BabyFloatingImage[Baby->m_NumBalloons - 1],
                          Baby->m_Pos.x, Baby->m_Pos.y, -46, 46, 0.0f);
      break;

    case BS_FLOATINGRIGHT:

      XEngine->DrawSprite(m_BabyFloatingImage[Baby->m_NumBalloons - 1],
                          Baby->m_Pos.x, Baby->m_Pos.y, 46, 46, 0.0f);
      break;

    case BS_BOUNCING:

      XEngine->DrawSpriteOC(m_BabyDeadImage, Baby->m_Pos.x, Baby->m_Pos.y, 46,
                            46, Baby->m_Rot);
      break;

    case BS_GETTINGUP:

      XEngine->DrawSpriteOC(m_BabyGetUpImage[Baby->m_WalkFrame], Baby->m_Pos.x,
                            Baby->m_Pos.y, 48.0f, 48.0f, Baby->m_Rot);
      break;

    case BS_WALKLEFT:

      XEngine->DrawSpriteOC(m_BabyWalkImage[Baby->m_WalkFrame], Baby->m_Pos.x,
                            Baby->m_Pos.y, -48, 48, 0.0f);
      break;

    case BS_WALKRIGHT:

      XEngine->DrawSpriteOC(m_BabyWalkImage[Baby->m_WalkFrame], Baby->m_Pos.x,
                            Baby->m_Pos.y, 48, 48, 0.0f);
      break;
    }

    if (Baby->m_HasBomb) {
      XEngine->DrawSprite(m_BombImage, Baby->m_Pos.x, Baby->m_Pos.y + 24.0f,
                          40.0f + 8.0f * Baby->m_ThreatLevel,
                          40.0f + 8.0f * Baby->m_ThreatLevel, 0.0f);
    }
  }

  XEngine->SetSpriteMode(SM_ALPHA_ADD);
  XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);

  for (std::list<CBaby *>::iterator iBaby = m_BabyList.begin(),
                                    iBaby_next = iBaby;
       iBaby != m_BabyList.end(); iBaby = iBaby_next) {
    iBaby_next = iBaby;
    ++iBaby_next;
    CBaby *Baby = (*iBaby);

    if (Baby->m_Hit) {
      switch (Baby->m_BabyState) {
      case BS_FLOATINGLEFT:

        XEngine->DrawSprite(m_BabyFloatingImage[Baby->m_NumBalloons - 1],
                            Baby->m_Pos.x, Baby->m_Pos.y, -46, 46, 0.0f);
        XEngine->DrawSprite(m_BabyFloatingImage[Baby->m_NumBalloons - 1],
                            Baby->m_Pos.x, Baby->m_Pos.y, -46, 46, 0.0f);
        XEngine->DrawSprite(m_BabyFloatingImage[Baby->m_NumBalloons - 1],
                            Baby->m_Pos.x, Baby->m_Pos.y, -46, 46, 0.0f);
        break;

      case BS_FLOATINGRIGHT:

        XEngine->DrawSprite(m_BabyFloatingImage[Baby->m_NumBalloons - 1],
                            Baby->m_Pos.x, Baby->m_Pos.y, 46, 46, 0.0f);
        XEngine->DrawSprite(m_BabyFloatingImage[Baby->m_NumBalloons - 1],
                            Baby->m_Pos.x, Baby->m_Pos.y, 46, 46, 0.0f);
        XEngine->DrawSprite(m_BabyFloatingImage[Baby->m_NumBalloons - 1],
                            Baby->m_Pos.x, Baby->m_Pos.y, 46, 46, 0.0f);
        break;

      case BS_BOUNCING:

        XEngine->DrawSpriteOC(m_BabyDeadImage, Baby->m_Pos.x, Baby->m_Pos.y, 46,
                              46, Baby->m_Rot);
        XEngine->DrawSpriteOC(m_BabyDeadImage, Baby->m_Pos.x, Baby->m_Pos.y, 46,
                              46, Baby->m_Rot);
        XEngine->DrawSpriteOC(m_BabyDeadImage, Baby->m_Pos.x, Baby->m_Pos.y, 46,
                              46, Baby->m_Rot);

        break;

      case BS_GETTINGUP:

        XEngine->DrawSpriteOC(m_BabyGetUpImage[Baby->m_WalkFrame],
                              Baby->m_Pos.x, Baby->m_Pos.y, 48.0f, 48.0f,
                              Baby->m_Rot);
        XEngine->DrawSpriteOC(m_BabyGetUpImage[Baby->m_WalkFrame],
                              Baby->m_Pos.x, Baby->m_Pos.y, 48.0f, 48.0f,
                              Baby->m_Rot);
        XEngine->DrawSpriteOC(m_BabyGetUpImage[Baby->m_WalkFrame],
                              Baby->m_Pos.x, Baby->m_Pos.y, 48.0f, 48.0f,
                              Baby->m_Rot);

        break;

      case BS_WALKLEFT:

        XEngine->DrawSpriteOC(m_BabyWalkImage[Baby->m_WalkFrame], Baby->m_Pos.x,
                              Baby->m_Pos.y, -48, 48, 0.0f);
        XEngine->DrawSpriteOC(m_BabyWalkImage[Baby->m_WalkFrame], Baby->m_Pos.x,
                              Baby->m_Pos.y, -48, 48, 0.0f);
        XEngine->DrawSpriteOC(m_BabyWalkImage[Baby->m_WalkFrame], Baby->m_Pos.x,
                              Baby->m_Pos.y, -48, 48, 0.0f);

        break;

      case BS_WALKRIGHT:

        XEngine->DrawSpriteOC(m_BabyWalkImage[Baby->m_WalkFrame], Baby->m_Pos.x,
                              Baby->m_Pos.y, 48, 48, 0.0f);
        XEngine->DrawSpriteOC(m_BabyWalkImage[Baby->m_WalkFrame], Baby->m_Pos.x,
                              Baby->m_Pos.y, 48, 48, 0.0f);
        XEngine->DrawSpriteOC(m_BabyWalkImage[Baby->m_WalkFrame], Baby->m_Pos.x,
                              Baby->m_Pos.y, 48, 48, 0.0f);

        break;
      }
    }
  }

  XEngine->SetSpriteMode(SM_ALPHA_BLEND);
  XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);

  for (std::list<CBomb *>::iterator iCBomb = m_BombList.begin(),
                                    iCBomb_next = iCBomb;
       iCBomb != m_BombList.end(); iCBomb = iCBomb_next) {
    iCBomb_next = iCBomb;
    ++iCBomb_next;
    CBomb *MyBomb = (*iCBomb);
    XEngine->DrawSprite(m_BombImage, MyBomb->m_Pos.x, MyBomb->m_Pos.y,
                        40.0f + 8.0f * MyBomb->m_Size,
                        40.0f + 8.0f * MyBomb->m_Size, MyBomb->m_Rot);
  }

  for (std::list<CPlayer *>::iterator iPlayer = m_PlayerList.begin(),
                                      iPlayer_next = iPlayer;
       iPlayer != m_PlayerList.end(); iPlayer = iPlayer_next) {
    iPlayer_next = iPlayer;
    ++iPlayer_next;
    CPlayer *Player = (*iPlayer);

    if (Player->m_PlayerCondition != PC_DEAD) {
      XEngine->DrawSprite(m_AimerImage[Player->m_ControllerNum],
                          Player->m_AimerPos.x, Player->m_AimerPos.y, 32, 32,
                          0);
    }

    float DrawAngle;
    int DrawImage = 6;

    if (Player->m_Direction < 0) {
      DrawAngle = -(270.0f - Player->m_Rot - 45.0f);

      if (Player->m_PlayerCondition != PC_DEAD) {
        XEngine->DrawSprite(
            m_PlayerImage[Player->m_ControllerNum][Player->m_CurrentFrame],
            Player->m_PlayerPos.x, Player->m_PlayerPos.y, -80, 80, 0);
        XEngine->DrawSprite(m_PlayerImage[Player->m_ControllerNum][DrawImage],
                            Player->m_PlayerPos.x, Player->m_PlayerPos.y, -80,
                            80, DrawAngle);
      }

      if (Player->m_PlayerCondition == PC_INVINCIBLE) {
        XEngine->SetSpriteMode(SM_ALPHA_ADD);
        XEngine->SetMaterial(
            1.0f, 1.0f, 1.0f,
            (Sin(Player->m_ConditionTime * 8.0f) / 2.0f + 0.5f));

        XEngine->DrawSprite(
            m_PlayerImage[Player->m_ControllerNum][Player->m_CurrentFrame],
            Player->m_PlayerPos.x, Player->m_PlayerPos.y, -80, 80, 0);
        XEngine->DrawSprite(m_PlayerImage[Player->m_ControllerNum][DrawImage],
                            Player->m_PlayerPos.x, Player->m_PlayerPos.y, -80,
                            80, DrawAngle);

        XEngine->SetSpriteMode(SM_ALPHA_BLEND);
        XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
      }

      if (Player->m_Weapon->m_WeaponType == WT_PLASMA &&
          Player->m_Weapon->m_Charge > 0.0f) {
        float Glow = (Sin(XEngine->m_Ticks * 64.0f) / 2.0f + 0.5f) *
                     Player->m_Weapon->m_Charge / 100.0f;

        XEngine->SetSpriteMode(SM_ALPHA_ADD);
        XEngine->SetMaterial(1.0f, 1.0f, 1.0f, Glow);

        XEngine->DrawSprite(m_PlayerImage[2][Player->m_CurrentFrame],
                            Player->m_PlayerPos.x, Player->m_PlayerPos.y, -80,
                            80, 0);
        XEngine->DrawSprite(m_PlayerImage[2][DrawImage], Player->m_PlayerPos.x,
                            Player->m_PlayerPos.y, -80, 80, DrawAngle);

        XEngine->SetSpriteMode(SM_ALPHA_BLEND);
        XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
      }
    } else {
      DrawAngle = 270.0f + Player->m_Rot - 45.0f;

      if (Player->m_PlayerCondition != PC_DEAD) {
        XEngine->DrawSprite(
            m_PlayerImage[Player->m_ControllerNum][Player->m_CurrentFrame],
            Player->m_PlayerPos.x, Player->m_PlayerPos.y, 80, 80, 0);
        XEngine->DrawSprite(m_PlayerImage[Player->m_ControllerNum][DrawImage],
                            Player->m_PlayerPos.x, Player->m_PlayerPos.y, 80,
                            80, DrawAngle);
      }

      if (Player->m_PlayerCondition == PC_INVINCIBLE) {
        XEngine->SetSpriteMode(SM_ALPHA_ADD);
        XEngine->SetMaterial(
            1.0f, 1.0f, 1.0f,
            (Sin(Player->m_ConditionTime * 8.0f) / 2.0f + 0.5f));

        XEngine->DrawSprite(
            m_PlayerImage[Player->m_ControllerNum][Player->m_CurrentFrame],
            Player->m_PlayerPos.x, Player->m_PlayerPos.y, 80, 80, 0);
        XEngine->DrawSprite(m_PlayerImage[Player->m_ControllerNum][DrawImage],
                            Player->m_PlayerPos.x, Player->m_PlayerPos.y, 80,
                            80, DrawAngle);

        XEngine->SetSpriteMode(SM_ALPHA_BLEND);
        XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
      }

      if (Player->m_Weapon->m_WeaponType == WT_PLASMA &&
          Player->m_Weapon->m_Charge > 0.0f) {
        float Glow = (Sin(XEngine->m_Ticks * 64.0f) / 2.0f + 0.5f) *
                     Player->m_Weapon->m_Charge / 100.0f;

        XEngine->SetSpriteMode(SM_ALPHA_ADD);
        XEngine->SetMaterial(1.0f, 1.0f, 1.0f, Glow);

        XEngine->DrawSprite(m_PlayerImage[2][Player->m_CurrentFrame],
                            Player->m_PlayerPos.x, Player->m_PlayerPos.y, 80,
                            80, 0);
        XEngine->DrawSprite(m_PlayerImage[2][DrawImage], Player->m_PlayerPos.x,
                            Player->m_PlayerPos.y, 80, 80, DrawAngle);

        XEngine->SetSpriteMode(SM_ALPHA_BLEND);
        XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
      }
    }

    if (Player->m_Weapon->m_WeaponType == WT_MACHINEGUN) {
      CMachineGun *MachineGun = (CMachineGun *)Player->m_Weapon;

      if (MachineGun->m_FlashTime < 3.0f) {
        XEngine->SetSpriteMode(SM_ALPHA_ADD);

        float Alpha = (2.0f - MachineGun->m_FlashTime * 1.25f);
        if (Alpha < 0.0f)
          Alpha = 0.0f;

        float Angle = Player->m_Rot - 135.0f;

        XEngine->SetMaterial(1.0f, 1.0f, 1.0f, Alpha);

        float DX = Player->m_AimerPos.x - MachineGun->m_HotPoint.x;
        float DY = Player->m_AimerPos.y - MachineGun->m_HotPoint.y;
        float OffsetLength = Sqrt(DX * DX + DY * DY);

        float PosX = MachineGun->m_HotPoint.x + DX / OffsetLength * 24.0f;
        float PosY = MachineGun->m_HotPoint.y + DY / OffsetLength * 24.0f;

        float Scale = 91.0f + Sin(XEngine->m_Ticks*31.f+Player->m_ControllerNum*57.f)*16.f;

        XEngine->DrawSprite(m_MuzzleFlashImage[MachineGun->m_FlashType], PosX,
                            PosY, Scale, Scale, Angle);

        XEngine->SetSpriteMode(SM_ALPHA_BLEND);
        XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
      }
    }
  }

  for (std::list<Gib *>::iterator iGib = m_GibList.begin(), iGib_next = iGib;
       iGib != m_GibList.end(); iGib = iGib_next) {
    iGib_next = iGib;
    ++iGib_next;
    Gib *MyGib = (*iGib);

    if (MyGib->m_Life < 30.0f) {
      XEngine->SetMaterial(1.0f, 1.0f, 1.0f, MyGib->m_Life / 30.0f);
    }

    XEngine->DrawSprite(m_GibImage[MyGib->m_GibType], MyGib->m_Pos.x,
                        MyGib->m_Pos.y, 16, 16, MyGib->m_Rot);

    if (MyGib->m_Life < 30.0f) {
      XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
    }
  }

  for (std::list<CWeaponBox *>::iterator iBox = m_BoxList.begin(),
                                         iBox_next = iBox;
       iBox != m_BoxList.end(); iBox = iBox_next) {
    iBox_next = iBox;
    ++iBox_next;
    CWeaponBox *WeaponBox = (*iBox);

    if (WeaponBox->m_Life < 60.0f) {
      XEngine->SetMaterial(1.0f, 1.0f, 1.0f, WeaponBox->m_Life / 60.0f);
    }

    if (WeaponBox->m_Special) {
      XEngine->DrawSprite(m_SpecialBoxImage[WeaponBox->m_WeaponType],
                          WeaponBox->m_Pos.x, WeaponBox->m_Pos.y, 40, 40, 0);
    } else {
      XEngine->DrawSprite(m_WeaponBoxImage[WeaponBox->m_WeaponType],
                          WeaponBox->m_Pos.x, WeaponBox->m_Pos.y, 40, 40, 0);
    }

    if (WeaponBox->m_Life < 60.0f) {
      XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);
    }

    if (WeaponBox->m_BoxState == WBS_FLOATING) {
      XEngine->DrawSprite(m_WeaponParachuteImage, WeaponBox->m_Pos.x,
                          WeaponBox->m_Pos.y - 26.0f, 40, 40, 0);
    }
  }

  for (std::list<Grenade *>::iterator iGrenade = Game.m_GrenadeList.begin(),
                                      iGrenade_next = iGrenade;
       iGrenade != Game.m_GrenadeList.end(); iGrenade = iGrenade_next) {
    iGrenade_next = iGrenade;
    ++iGrenade_next;
    Grenade *MyGrenade = (*iGrenade);
    XEngine->DrawSprite(m_GrenadeImage, MyGrenade->m_Pos.x, MyGrenade->m_Pos.y,
                        32, 32, MyGrenade->m_Rot);
  }

  for (std::list<CRocket *>::iterator iRocket = m_RocketList.begin(),
                                      iRocket_next = iRocket;
       iRocket != m_RocketList.end(); iRocket = iRocket_next) {
    iRocket_next = iRocket;
    ++iRocket_next;
    CRocket *Rocket = (*iRocket);
    XEngine->DrawSprite(m_RocketImage, Rocket->m_Pos.x, Rocket->m_Pos.y, 32, 32,
                        Rocket->m_Rot);
  }

  for (std::list<Smoke *>::iterator iSmoke = Game.m_SmokeList.begin(),
                                    iSmoke_next = iSmoke;
       iSmoke != Game.m_SmokeList.end(); iSmoke = iSmoke_next) {
    iSmoke_next = iSmoke;
    ++iSmoke_next;
    Smoke *MySmoke = (*iSmoke);

    XEngine->SetMaterial(1.0f, 1.0f, 1.0f,
                         (25.0f - fAbs((50.0f - MySmoke->m_Life) - 25.0f)) /
                             50.0f);

    if (MySmoke->m_RocketSmoke) {
      XEngine->DrawSprite(m_RocketSmokeImage, MySmoke->m_Pos.x,
                          MySmoke->m_Pos.y, (100.0f - MySmoke->m_Life) / 2.0f,
                          (100.0f - MySmoke->m_Life) / 2.0f, MySmoke->m_Rot);
    } else {
      XEngine->DrawSprite(m_SmokeImage, MySmoke->m_Pos.x, MySmoke->m_Pos.y,
                          (100.0f - MySmoke->m_Life) / 2.0f,
                          (100.0f - MySmoke->m_Life) / 2.0f, MySmoke->m_Rot);
    }
  }

  XEngine->SetSpriteMode(SM_ALPHA_ADD);

  for (std::list<Plasma *>::iterator iPlasma = m_PlasmaList.begin(),
                                     iPlasma_next = iPlasma;
       iPlasma != m_PlasmaList.end(); iPlasma = iPlasma_next) {
    iPlasma_next = iPlasma;
    ++iPlasma_next;
    Plasma *MyPlasma = (*iPlasma);

    float TrailLength = 60.0f - MyPlasma->m_Life;

    if (TrailLength > 30.0f) {
      TrailLength = 30.0f;
    }

    for (float Delta = 0.0f; Delta < TrailLength; Delta += 1.0f) {
      XEngine->SetMaterial(1.0f, 1.0f, 1.0f, (30.0f - Delta) / 30.0f);
      XEngine->DrawSprite(
          m_PlasmaImage,
          MyPlasma->m_Pos.x -
              MyPlasma->m_Speed.x * Delta / 10.0f * MyPlasma->m_PowerLevel,
          MyPlasma->m_Pos.y -
              MyPlasma->m_Speed.y * Delta / 10.0f * MyPlasma->m_PowerLevel,
          32.0f, 32.0f, MyPlasma->m_Rot);
    }
  }

  XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);

  for (std::list<Bullet *>::iterator iBullet = Game.m_BulletList.begin(),
                                     iBullet_next = iBullet;
       iBullet != Game.m_BulletList.end(); iBullet = iBullet_next) {
    iBullet_next = iBullet;
    ++iBullet_next;
    Bullet *MyBullet = (*iBullet);
    XEngine->DrawTracer(m_BulletTracerImage, MyBullet->m_oPos.x,
                        MyBullet->m_oPos.y, MyBullet->m_Pos.x,
                        MyBullet->m_Pos.y, 3.0f);
  }

  for (std::list<BulletSpark *>::iterator
           iBulletSpark = Game.m_BulletSparkList.begin(),
           iBulletSpark_next = iBulletSpark;
       iBulletSpark != Game.m_BulletSparkList.end();
       iBulletSpark = iBulletSpark_next) {
    iBulletSpark_next = iBulletSpark;
    ++iBulletSpark_next;
    BulletSpark *MyBulletSpark = (*iBulletSpark);

    XEngine->SetMaterial(1.0f, 1.0f, 1.0f, MyBulletSpark->m_Life / 10.0f);
    XEngine->DrawSprite(m_FireImage, MyBulletSpark->m_Pos.x,
                        MyBulletSpark->m_Pos.y, 8.0f, 8.0f, 0.0f);
    XEngine->DrawSprite(m_FireImage, MyBulletSpark->m_Pos.x,
                        MyBulletSpark->m_Pos.y, 8.0f, 8.0f, 0.0f);
  }

  for (std::list<CWeaponBox *>::iterator iBox = m_BoxList.begin(),
                                         iBox_next = iBox;
       iBox != m_BoxList.end(); iBox = iBox_next) {
    iBox_next = iBox;
    ++iBox_next;
    CWeaponBox *WeaponBox = (*iBox);

    float Alpha;

    if (WeaponBox->m_Life > 60.0f) {
      Alpha = 1.0f;
    } else {
      Alpha = WeaponBox->m_Life / 60.0f;
    }

    XEngine->SetMaterial(1.0f, 1.0f, 1.0f,
                         (Sin(WeaponBox->m_Rot * 4.0f) / 2.0f + 0.5f) * Alpha);

    if (WeaponBox->m_Special) {
      XEngine->DrawSprite(m_SpecialBoxImage[WeaponBox->m_WeaponType],
                          WeaponBox->m_Pos.x, WeaponBox->m_Pos.y, 40, 40, 0);
    } else {
      XEngine->DrawSprite(m_WeaponBoxImage[WeaponBox->m_WeaponType],
                          WeaponBox->m_Pos.x, WeaponBox->m_Pos.y, 40, 40, 0);
    }
  }

  for (std::list<Slug *>::iterator iSlug = Game.m_SlugList.begin(),
                                   iSlug_next = iSlug;
       iSlug != Game.m_SlugList.end(); iSlug = iSlug_next) {
    iSlug_next = iSlug;
    ++iSlug_next;
    Slug *MySlug = (*iSlug);

    XEngine->SetMaterial(1.0f, 1.0f, 1.0f, MySlug->m_Life / 30.0f);

    XEngine->DrawTracer(m_SlugTracerImage, MySlug->m_Pos.x, MySlug->m_Pos.y,
                        MySlug->m_Pos.x + MySlug->m_Speed.x,
                        MySlug->m_Pos.y + MySlug->m_Speed.y, 10.0f);
    XEngine->DrawTracer(m_SlugTracerImage, MySlug->m_Pos.x, MySlug->m_Pos.y,
                        MySlug->m_Pos.x + MySlug->m_Speed.x,
                        MySlug->m_Pos.y + MySlug->m_Speed.y, 10.0f);
    XEngine->DrawTracer(m_BulletTracerImage, MySlug->m_Pos.x, MySlug->m_Pos.y,
                        MySlug->m_Pos.x + MySlug->m_Speed.x,
                        MySlug->m_Pos.y + MySlug->m_Speed.y, 4.0f);
    XEngine->DrawTracer(m_BulletTracerImage, MySlug->m_Pos.x, MySlug->m_Pos.y,
                        MySlug->m_Pos.x + MySlug->m_Speed.x,
                        MySlug->m_Pos.y + MySlug->m_Speed.y, 4.0f);
  }

  for (std::list<Fire *>::iterator iFire = Game.m_FireList.begin(),
                                   iFire_next = iFire;
       iFire != Game.m_FireList.end(); iFire = iFire_next) {
    iFire_next = iFire;
    ++iFire_next;
    Fire *MyFire = (*iFire);

    if (MyFire->m_SmallFire) {
      XEngine->SetMaterial(1.0f, 1.0f, 1.0f, MyFire->m_Life / 90.0f);
      XEngine->DrawSprite(m_FireImage, MyFire->m_Pos.x, MyFire->m_Pos.y,
                          (75.0f - MyFire->m_Life) / 2.0f,
                          (75.0f - MyFire->m_Life) / 2.0f, MyFire->m_Rot);
    } else {
      XEngine->SetMaterial(1.0f, 1.0f, 1.0f, MyFire->m_Life / 50.0f);
      XEngine->DrawSprite(m_FireImage, MyFire->m_Pos.x, MyFire->m_Pos.y,
                          (200.0f - MyFire->m_Life) / 2.0f,
                          (200.0f - MyFire->m_Life) / 2.0f, MyFire->m_Rot);
    }
  }

  for (std::list<Flame *>::iterator iFlame = Game.m_FlameList.begin(),
                                    iFlame_next = iFlame;
       iFlame != Game.m_FlameList.end(); iFlame = iFlame_next) {
    iFlame_next = iFlame;
    ++iFlame_next;
    Flame *MyFlame = (*iFlame);

    XEngine->SetMaterial(1.0f, 1.0f, 1.0f, MyFlame->m_Life / 45.0f);

    XEngine->DrawSprite(
        m_FireImage, MyFlame->m_Pos.x, MyFlame->m_Pos.y,
        100.0f - MyFlame->m_Life * 3.0f + MyFlame->m_PowerLevel * 25,
        100.0f - MyFlame->m_Life * 3.0f + MyFlame->m_PowerLevel * 25,
        MyFlame->m_Rot);
  }

  for (std::list<Spark *>::iterator iSpark = Game.m_SparkList.begin(),
                                    iSpark_next = iSpark;
       iSpark != Game.m_SparkList.end(); iSpark = iSpark_next) {
    iSpark_next = iSpark;
    ++iSpark_next;
    Spark *MySpark = (*iSpark);

    XEngine->SetMaterial(1.0f, 1.0f, 1.0f, MySpark->m_Life / 16.0f);
    XEngine->DrawSprite(m_SparkImage, MySpark->m_Pos.x, MySpark->m_Pos.y, 8.0f,
                        8.0f, MySpark->m_Rot);
  }

  for (std::list<Corona *>::iterator iCorona = Game.m_CoronaList.begin(),
                                     iCorona_next = iCorona;
       iCorona != Game.m_CoronaList.end(); iCorona = iCorona_next) {
    iCorona_next = iCorona;
    ++iCorona_next;
    Corona *MyCorona = (*iCorona);

    XEngine->SetMaterial(1.0f, 1.0f, 1.0f, MyCorona->m_Life / 20.0f);
    XEngine->DrawSprite(m_CoronaImage, MyCorona->m_Pos.x, MyCorona->m_Pos.y,
                        75.0f, 75.0f, MyCorona->m_Rot);
  }

  XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 0.5f);

  for (std::list<CRocket *>::iterator iRocket = m_RocketList.begin(),
                                      iRocket_next = iRocket;
       iRocket != m_RocketList.end(); iRocket = iRocket_next) {
    iRocket_next = iRocket;
    ++iRocket_next;
    CRocket *Rocket = (*iRocket);

    float FlarePosX, FlarePosY;

    FlarePosX = Rocket->m_Pos.x + Sin(Rocket->m_Rot) * -6.0f;
    FlarePosY = Rocket->m_Pos.y + Cos(Rocket->m_Rot) * -6.0f;

    XEngine->DrawSprite(m_FireImage, FlarePosX, FlarePosY, 16, 16,
                        Rocket->m_Rot);
  }

  for (std::list<Score *>::iterator iScore = m_ScoreList.begin(),
                                    iScore_next = iScore;
       iScore != m_ScoreList.end(); iScore = iScore_next) {
    iScore_next = iScore;
    ++iScore_next;
    Score *MyScore = (*iScore);

    char ScoreText[256];
    sprintf(ScoreText, "%i", MyScore->m_Value);

    XEngine->DrawScoreText(ScoreText, (int)MyScore->m_Pos.x,
                           (int)MyScore->m_Pos.y, MyScore->m_Red,
                           MyScore->m_Green, MyScore->m_Blue);
  }

  // Draw GUI
  XEngine->SetCameraPos(0.0f, 0.0f);

  XEngine->SetSpriteMode(SM_ALPHA_BLEND);
  XEngine->SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);

  if (m_Paused) {
    XEngine->DrawText("PAUSED", 280, 240, 255, 255, 255);
  }

  for (std::list<CPlayer *>::iterator iPlayer = m_PlayerList.begin(),
                                      iPlayer_next = iPlayer;
       iPlayer != m_PlayerList.end(); iPlayer = iPlayer_next) {
    iPlayer_next = iPlayer;
    ++iPlayer_next;
    CPlayer *Player = (*iPlayer);

    char PlayerScore[256];
    sprintf(PlayerScore, "Score:%i", Player->m_Score);
    char powerLabel[24];
    snprintf(powerLabel, sizeof(powerLabel), "L%d/%d", Player->m_Weapon->m_PowerLevel, WeaponPowerLimit());
    XEngine->DrawUiText(powerLabel, 125 + Player->m_ControllerNum * 155, 72, 8, 220, 210, 190, true);


    switch (Player->m_ControllerNum) {
    case 0:

      XEngine->DrawSprite(m_GUIBoxImage, 85, 22, 256, 128, 0);
      XEngine->DrawHudScore(PlayerScore, 85, 255, 50, 50);

      for (int i = 0; i < Player->m_Lives; i++) {
        XEngine->DrawSprite(m_PlayerLivesImage[0], float(25 + i * 28), 50, 32,
                            32, 0);
      }

      XEngine->DrawSprite(m_GUIBoxImage, 125, 50, 50, 80, 0);
      XEngine->DrawTracer(
          m_PowerBarImage, 145,
          float(60 - Player->m_Weapon->m_PowerLevel * (20.f / WeaponPowerLimit())), 145, 60, 5);
      XEngine->DrawTracer(m_PowerBarBoxImage, 145, 40.0f, 145, 60.0f, 5);

      XEngine->DrawSprite(m_WeaponBoxImage[Player->m_Weapon->m_WeaponType], 125,
                          50, 32, 32, 0);

      break;

    case 1:

      XEngine->DrawSprite(m_GUIBoxImage, 240, 22, 256, 128, 0);
      XEngine->DrawHudScore(PlayerScore, 240, 255, 255, 0);

      for (int i = 0; i < Player->m_Lives; i++) {
        XEngine->DrawSprite(m_PlayerLivesImage[1], float(180 + i * 28), 50, 32,
                            32, 0);
      }

      XEngine->DrawSprite(m_GUIBoxImage, 280, 50, 50, 80, 0);
      XEngine->DrawTracer(
          m_PowerBarImage, 300,
          float(60 - Player->m_Weapon->m_PowerLevel * (20.f / WeaponPowerLimit())), 300, 60, 5);
      XEngine->DrawTracer(m_PowerBarBoxImage, 300, 40.0f, 300, 60.0f, 5);

      XEngine->DrawSprite(m_WeaponBoxImage[Player->m_Weapon->m_WeaponType], 280,
                          50, 32, 32, 0);

      break;

    case 2:

      XEngine->DrawSprite(m_GUIBoxImage, 395, 22, 256, 128, 0);
      XEngine->DrawHudScore(PlayerScore, 395, 100, 100, 255);

      for (int i = 0; i < Player->m_Lives; i++) {
        XEngine->DrawSprite(m_PlayerLivesImage[2], float(335 + i * 28), 50, 32,
                            32, 0);
      }

      XEngine->DrawSprite(m_GUIBoxImage, 425, 50, 50, 80, 0);
      XEngine->DrawTracer(
          m_PowerBarImage, 445,
          float(60 - Player->m_Weapon->m_PowerLevel * (20.f / WeaponPowerLimit())), 445, 60, 5);
      XEngine->DrawTracer(m_PowerBarBoxImage, 445, 40.0f, 445, 60.0f, 5);

      XEngine->DrawSprite(m_WeaponBoxImage[Player->m_Weapon->m_WeaponType], 425,
                          50, 32, 32, 0);

      break;

    case 3:

      XEngine->DrawSprite(m_GUIBoxImage, 550, 22, 256, 128, 0);
      XEngine->DrawHudScore(PlayerScore, 550, 0, 255, 0);

      for (int i = 0; i < Player->m_Lives; i++) {
        XEngine->DrawSprite(m_PlayerLivesImage[3], float(490 + i * 28), 50, 32,
                            32, 0);
      }

      XEngine->DrawSprite(m_GUIBoxImage, 590, 50, 50, 80, 0);
      XEngine->DrawTracer(
          m_PowerBarImage, 610,
          float(60 - Player->m_Weapon->m_PowerLevel * (20.f / WeaponPowerLimit())), 610, 60, 5);
      XEngine->DrawTracer(m_PowerBarBoxImage, 610, 40.0f, 610, 60.0f, 5);

      XEngine->DrawSprite(m_WeaponBoxImage[Player->m_Weapon->m_WeaponType], 590,
                          50, 32, 32, 0);

      break;
    }
  }

  if (m_ResultsReady) { DrawResults(); return; }

  float Alpha = (120.0f - m_GameOverDelay) / 120.0f;

  if (Alpha > 1.0f) {
    Alpha = 1.0f;
  }

  if (Alpha < 0.0f) {
    Alpha = 0.0f;
  }

  XEngine->SetSpriteMode(SM_ALPHA_BLEND);
  XEngine->SetMaterial(1.0f, 1.0f, 1.0f, Alpha);
  XEngine->DrawSprite(m_GameOverImage, 320.0f, 240.0f, 640, 640.0f, 0);
}

// Results use round-owned players, including those eliminated earlier.
void CGame::FinishRound() {
  m_ResultsReady = true;
  m_ResultsTime = 0;
  m_ResultsStarted = GetTickCount64();
  XEngine->SetGameSpeed(1);
  XEngine->BeginResultsAudio();
  for (const auto &p : ownedPlayers) {
    int slot = p->m_ControllerNum;
    m_Participants |= 1u << slot;
    m_FinalScores[slot] = std::max(m_BestScores[slot], p->m_Score);
  }
  if (getenv("DBX_TEST_GAMEOVER_AT")) {
    if (const char *scores = getenv("DBX_TEST_RESULTS"))
      sscanf(scores, "%d,%d,%d,%d", &m_FinalScores[0], &m_FinalScores[1],
             &m_FinalScores[2], &m_FinalScores[3]);
  }
  m_ResultOrder = NativeResultOrder(m_Participants, m_FinalScores);
  // A small text file in the user's application data survives upgrades.
  std::string path = "highscore.txt";
  FILE *f = fopen(path.c_str(), "r");
  if (f) {
    int value = 0;
    if (fscanf(f, "%d", &value) == 1)
      m_HighScore = std::max(0, value);
    fclose(f);
  }
  bool solo = m_Participants && !(m_Participants & (m_Participants - 1));
  if (solo) {
    int score = 0;
    for (int i = 0; i < 4; ++i)
      if (m_Participants & (1u << i))
        score = m_FinalScores[i];
    if (score > m_HighScore) {
      m_HighScore = score;
      m_NewHighScore = true;
      // Replace atomically so an interrupted save preserves the previous
      // record.
      std::string temp = path + ".tmp";
      f = fopen(temp.c_str(), "w");
      bool saved = false;
      if (f) {
        bool written = fprintf(f, "%d\n", m_HighScore) > 0;
        bool closed = fclose(f) == 0;
        saved = written && closed &&
                MoveFileExA(temp.c_str(), path.c_str(),
                            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
      }
      if (!saved)
        fprintf(stderr, "RESULTS high score save failed\n");
    }
  }
  fprintf(stderr, "RESULTS mask=%u scores=%d,%d,%d,%d high=%d\n",
          m_Participants, m_FinalScores[0], m_FinalScores[1], m_FinalScores[2],
          m_FinalScores[3], m_HighScore);
}

void CGame::DrawResults() {
  XEngine->SetCameraPos(0, 0);
  XEngine->SetSpriteMode(SM_ALPHA_BLEND);
  XEngine->SetMaterial(1, 1, 1, 1);
  XEngine->FillRect(-20,-20,680,520,.018f,.015f,.013f,.94f);
  XEngine->DrawUiText("GAME OVER",322,38,34,45,24,18,true);
  XEngine->DrawUiText("GAME OVER",320,36,34,244,228,200,true);
  XEngine->FillRect(266,82,108,2,.72f,.21f,.12f,1);
  XEngine->DrawUiText("ROUND COMPLETE",320,94,11,171,145,111,true);
  const auto &order = m_ResultOrder;
  const size_t count = order.size();
  const float t = m_ResultsTime / 60.f;
  if (!count) {
    XEngine->DrawUiText("NO SCORES RECORDED",320,240,18,200,187,165,true);
    if (NativeResultsCanContinue(count,t)) XEngine->DrawUiText("TAP / A: TITLE SCREEN",320,445,12,205,195,178,true);
    return;
  }
  const int colours[4][3]={{255,82,65},{255,216,73},{115,162,255},{110,226,127}};
  const bool locked = t >= NativeResultRollDuration(count);
  const int winner = order.front();
  const bool tied = count>1 && m_FinalScores[order[1]]==m_FinalScores[winner];
  char label[96];
  if (!locked) {
    XEngine->DrawUiText("ROLLING THE RESULTS...",320,119,21,194,175,147,true);
  } else {
    XEngine->DrawUiText(tied?"WINNERS!":"WINNER!",320,111,26,255,211,113,true);
    if(tied)snprintf(label,sizeof(label),"JOINT FIRST PLACE");
    else snprintf(label,sizeof(label),"PLAYER %d",winner+1);
    XEngine->DrawUiText(label,320,143,9,217,189,145,true);
  }

  const float cardW=count==4?116.f:count==3?140.f:count==2?164.f:220.f;
  const float gap=16.f, total=cardW*count+gap*(count-1);
  const float left=320-total/2;
  const float cursor=NativeResultCursor(count,t);
  const int active=int(std::lround(cursor));
  XEngine->FillRect(left-5,175,total+10,3,.26f,.19f,.12f,1);
  for(size_t row=0;row<count;++row) {
    int player=order[row];float x=left+row*(cardW+gap),center=x+cardW/2;
    bool top=m_FinalScores[player]==m_FinalScores[winner];
    bool lit=locked?top:int(row)==active;
    NativeUiPanel(x,192,cardW,190);
    if(lit) {
      XEngine->FillRect(x+2,194,cardW-4,2,1,.70f,.21f,1);
      XEngine->FillRect(x+2,378,cardW-4,2,1,.70f,.21f,1);
      XEngine->FillRect(x+2,196,2,182,.75f,.41f,.13f,1);
      XEngine->FillRect(x+cardW-4,196,2,182,.75f,.41f,.13f,1);
    }
    int rank=NativeResultRank(order,m_FinalScores,row);
    snprintf(label,sizeof(label),"RANK %d",rank);
    XEngine->DrawUiText(label,center,203,11,lit?255:151,lit?210:137,lit?117:116,true);
    XEngine->DrawSprite(m_PlayerImage[player][4],center,250,160,160,0);
    XEngine->DrawSprite(m_PlayerImage[player][5],center,250,160,160,0);
    snprintf(label,sizeof(label),"PLAYER %d",player+1);
    XEngine->DrawUiText(label,center,316,14,colours[player][0],colours[player][1],colours[player][2],true);
    XEngine->DrawUiText("SCORE",center,339,9,150,137,117,true);
    if(locked) snprintf(label,sizeof(label),"%d",NativeResultDisplayedScore(m_FinalScores[player],count,t));
    else snprintf(label,sizeof(label),"-----");
    float size=std::min(22.f,(cardW-20)/(std::strlen(label)*.66f));
    XEngine->DrawUiText(label,center,353,size,locked&&top?255:207,locked&&top?215:192,locked&&top?122:170,true);
  }
  float arrowX=left+cardW/2+cursor*(cardW+gap);
  NativeUiArrowDown(arrowX,173 + (locked?0.f:std::sin(t*9.f)*2.f));
  float progress=locked?1.f:std::clamp(t/NativeResultRollDuration(count),0.f,1.f);
  XEngine->FillRect(130,399,380,2,.16f,.12f,.09f,1);
  XEngine->FillRect(130,399,380*progress,2,.68f,.35f,.13f,1);
  if(locked && count==1) {
    snprintf(label,sizeof(label),m_NewHighScore?"NEW HIGH SCORE: %d":"HIGH SCORE: %d",m_HighScore);
    XEngine->DrawUiText(label,320,413,15,255,207,100,true);
  } else if(locked) {
    XEngine->DrawUiText(NativeResultScoreProgress(count,t)<1?"COUNTING SCORES...":"FINAL STANDINGS",320,413,11,190,162,125,true);
  }
  XEngine->DrawUiText(NativeResultsCanContinue(count,t)?"ENTER / A / START: TITLE SCREEN":"RESULTS REVEAL IN PROGRESS",320,447,11,194,180,157,true);
}
