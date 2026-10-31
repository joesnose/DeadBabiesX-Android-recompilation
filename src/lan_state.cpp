#include "lan.h"
#include "game.h"
#include "sync_rng.h"
#include <cstring>
namespace {
struct Hash {uint64_t h=1469598103934665603ull;void word(uint32_t n){for(int i=0;i<4;i++){h^=uint8_t(n>>(i*8));h*=1099511628211ull;}}void val(int n){word(uint32_t(n));}void val(unsigned n){word(n);}void val(bool n){word(n?1:0);}void val(float n){uint32_t v;memcpy(&v,&n,4);word(v);}void val(const D3DXVECTOR2&v){val(v.x);val(v.y);}template<class...T>void fields(const T&...v){(val(v),...);}template<class T>void size(const T&v){word(v.size());}};
}
uint64_t LanGameHash(){Hash h;h.fields(dbxRandomState,XEngine->m_Ticks,XEngine->m_GameSpeed,Game.m_NumPlayers,Game.m_NumBabies,Game.m_BabiesKilled,Game.m_BabyDelay,Game.m_BoxDelay,Game.m_Gravity,Game.m_Scroll,Game.m_ResultsReady,Game.m_ResultsTime);
 h.size(Game.m_PlayerList);for(auto*p:Game.m_PlayerList){h.fields(p->m_ControllerNum,p->m_PlayerPos,p->m_AimerPos,p->m_Score,p->m_Bonus,p->m_Lives,p->m_PlayerCondition,p->m_ConditionTime,p->m_PlayerState,p->m_FrameTime);if(p->m_Weapon)h.fields(p->m_Weapon->m_WeaponType,p->m_Weapon->m_PowerLevel,p->m_Weapon->m_Charge,p->m_Weapon->m_HotPoint);}
 h.size(Game.m_BabyList);for(auto*p:Game.m_BabyList)h.fields(p->m_Pos,p->m_Speed,p->m_Health,p->m_BabyState,p->m_NumBalloons,p->m_OnFire,p->m_HasBomb,p->m_ThreatLevel);
 h.size(Game.m_BoxList);for(auto*p:Game.m_BoxList)h.fields(p->m_Pos,p->m_Speed,p->m_Life,p->m_WeaponType,p->m_Special,p->m_BoxState);
 h.size(Game.m_RocketList);for(auto*p:Game.m_RocketList)h.fields(p->m_Pos,p->m_Speed,p->m_Fuel,p->m_PowerLevel);
 h.size(Game.m_BulletList);for(auto*p:Game.m_BulletList)h.fields(p->m_Pos,p->m_Speed,p->m_Life,p->m_PowerLevel);
 h.size(Game.m_PlasmaList);for(auto*p:Game.m_PlasmaList)h.fields(p->m_Pos,p->m_Speed,p->m_Life,p->m_PowerLevel);
 h.size(Game.m_GrenadeList);for(auto*p:Game.m_GrenadeList)h.fields(p->m_Pos,p->m_Speed,p->m_Life,p->m_PowerLevel);
 h.size(Game.m_FlameList);for(auto*p:Game.m_FlameList)h.fields(p->m_Pos,p->m_Speed,p->m_Life,p->m_PowerLevel);
 h.size(Game.m_BombList);for(auto*p:Game.m_BombList)h.fields(p->m_Pos,p->m_Speed,p->m_Size);
 h.size(Game.m_SpecialList);for(auto*p:Game.m_SpecialList)h.fields(p->m_Activated,p->m_Life,p->m_FireDelay);
 for(int i=0;i<4;i++)h.fields(Game.m_BestScores[i],Game.m_Continues[i],Game.m_FinalScores[i]);return h.h;
}
