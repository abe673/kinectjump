#pragma once
#include "raylib.h"

class SoundEffect {
public:
  void Init();
  void PlayLose();
  void PlayJump();
  void PlayLand();
  void PlayHit();
  void Unload();

private:
  Sound lose;
  Sound jump;
  Sound land;
  Sound hit;
};
