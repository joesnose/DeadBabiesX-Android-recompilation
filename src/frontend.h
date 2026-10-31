#pragma once
#include "game.h"
enum class MenuAction { None, Start, Resume, Title, ArtChanged, Exit };
class Frontend {
public:
  bool playing = false, paused = false;
  Frontend();
  void OpenPause();
  void OpenTitle();
  void Play();
  bool WantsPause();
  MenuAction Update();
  int PlayerCount() const { return players; }
  bool MegaWeapons() const { return megaWeapons; }
  void Draw();

private:
  int selection = 0, heldDirection = 0, heldHorizontal = 0, players = 1;
  bool inputArmed = false;
  int artMode = 0;
  bool megaWeapons = false;
  MenuAction ToggleArt(int delta);
  MenuAction ToggleMusic(int delta);
  IDirect3DTexture8 *backdrop = nullptr;
};
