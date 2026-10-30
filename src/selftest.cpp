#include "game.h"
#include <cmath>
int NativeSelfTest() {
  int failures = 0;
  auto check = [&](bool ok, const char *name) {
    fprintf(stderr, "%s: %s\n", ok ? "PASS" : "FAIL", name);
    failures += !ok;
  };
  auto hit = XEngine->Collide(0, 0, 1, 10, 0, 5, 0, 1);
  check(hit.Collided && std::abs(hit.Pos.x - 3) < 0.001f &&
            std::abs(hit.Pos.y) < 0.001f,
        "original swept collision response");
  auto miss = XEngine->Collide(0, 0, 1, 10, 0, 5, 5, 1);
  check(!miss.Collided && miss.Pos.x == 10, "swept collision miss");
  CPlayer player;
  player.m_AimerPos = {250, 100};
  CMachineGun gun;
  gun.m_Player = &player;
  gun.m_HotPoint = {100, 200};
  gun.Fire();
  check(Game.m_BulletList.size() == 1, "machine gun creates a projectile");
  if (!Game.m_BulletList.empty()) {
    Bullet *b = Game.m_BulletList.front();
    check(b->m_Pos.x == 100 && b->m_Pos.y == 200 && b->m_sPos.x == 100 &&
              b->m_sPos.y == 200,
          "machine gun fires from its moving muzzle");
    check(std::isfinite(b->m_Speed.x) && std::isfinite(b->m_Speed.y),
          "machine gun velocity is finite");
  }
  CFlameThrower flame;
  flame.m_Player = &player;
  flame.m_HotPoint = {120, 220};
  flame.Fire();
  check(Game.m_FlameList.size() == 1, "flamethrower creates a projectile");
  if (!Game.m_FlameList.empty()) {
    Flame *f = Game.m_FlameList.front();
    check(f->m_Pos.x == 120 && f->m_Pos.y == 220 && f->m_Player == &player,
          "flame retains its originating player and muzzle");
    check(std::isfinite(f->m_Speed.x) && std::isfinite(f->m_Speed.y),
          "flame velocity is finite");
  }
  SetMegaWeapons(false);
  gun.m_PowerLevel = 1;
  for (int i = 0; i < 8; ++i) gun.Upgrade();
  check(gun.m_PowerLevel == 3, "normal pickups stop at weapon level 3");
  SetMegaWeapons(true);
  for (int i = 0; i < 8; ++i) gun.Upgrade();
  check(gun.m_PowerLevel == 5, "mega pickups reach and stop at weapon level 5");
  check(GrenadeFragments(1, false) == 8 && GrenadeFragments(2, false) == 20 &&
        GrenadeFragments(3, false) == 32 && GrenadeFragments(3, true) == 60,
        "original grenade fragment counts preserved");
  check(BulletKnockbackDivisor(1) == 20 && BulletKnockbackDivisor(2) == 15 &&
        BulletKnockbackDivisor(3) == 10 && BulletKnockbackDivisor(4) == 7.5f &&
        BulletKnockbackDivisor(5) == 6, "machine gun impulse scales through level 5");
  for (int level = 4; level <= 5; ++level) {
    CPlasma plasma;
    plasma.m_Player = &player; plasma.m_HotPoint = {100, 200};
    plasma.m_PowerLevel = level;
    plasma.Fire();
    check(Game.m_PlasmaList.size() == size_t(level), "mega plasma emits every shot in its fan");
    bool moving = true;
    for (auto *p : Game.m_PlasmaList) {
      moving = moving && std::isfinite(p->m_Speed.x) && std::isfinite(p->m_Speed.y) &&
               std::abs(std::hypot(p->m_Speed.x, p->m_Speed.y) - 16) < .001f;
      delete p;
    }
    Game.m_PlasmaList.clear();
    check(moving, "level 4/5 plasma shots have finite full-speed velocities");
    CGrenades grenades;
    grenades.m_Player = &player; grenades.m_HotPoint = {100, 200};
    grenades.m_PowerLevel = level;
    grenades.Fire();
    check(Game.m_GrenadeList.size() == 1 && Game.m_GrenadeList.front()->m_PowerLevel == level,
          "mega grenade retains launcher power");
    size_t before = Game.m_BulletList.size();
    grenades.FireAlternate();
    check(Game.m_BulletList.size() - before == size_t(level * 20),
          "level 4/5 remote grenade produces 80/100 fragments");
    CRockets rockets;
    rockets.m_Player = &player; rockets.m_HotPoint = {100, 200};
    rockets.m_PowerLevel = level;
    rockets.Fire();
    check(Game.m_RocketList.size() == size_t(level), "mega rocket volley contains 4/5 rockets");
    for (auto *p : Game.m_RocketList) delete p;
    Game.m_RocketList.clear();
    CRailgun rail;
    rail.m_Player = &player; rail.m_HotPoint = {100, 200}; rail.m_PowerLevel = level;
    rail.Fire();
    check(std::abs(rail.m_FireDelay - 30.f / level) < .001f,
          "mega railgun cooldown continues to scale");
    size_t flameBefore = Game.m_FlameList.size();
    CFlameThrower megaFlame;
    megaFlame.m_Player = &player; megaFlame.m_HotPoint = {100, 200}; megaFlame.m_PowerLevel = level;
    megaFlame.Fire();
    check(Game.m_FlameList.size() == flameBefore + 1 &&
          Game.m_FlameList.back()->m_Life == 20 + level * 5 &&
          Game.m_FlameList.back()->m_PowerLevel == level,
          "mega flame range/lifetime scales to levels 4/5");
    megaFlame.Stop();
  }
  SetMegaWeapons(false);
  flame.Stop();
  for (auto *p : Game.m_BulletList)
    delete p;
  Game.m_BulletList.clear();
  for (auto *p : Game.m_FlameList)
    delete p;
  Game.m_FlameList.clear();
  for (auto *p : Game.m_FireList)
    delete p;
  Game.m_FireList.clear();
  for (auto *p : Game.m_SmokeList) delete p;
  Game.m_SmokeList.clear();
  for (auto *p : Game.m_ScreenShakeList) delete p;
  Game.m_ScreenShakeList.clear();
  for (auto *p : Game.m_SlugList) delete p;
  Game.m_SlugList.clear();
  for (auto *p : Game.m_CoronaList) delete p;
  Game.m_CoronaList.clear();
  fprintf(stderr, "Self-test: %d failures\n", failures);
  return failures ? 1 : 0;
}
