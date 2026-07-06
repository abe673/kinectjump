#include "SoundEffect.h"
void SoundEffect::Init() {
  lose = LoadSound("resources/sound/lose.mp3");
  jump = LoadSound("resources/sound/jump.mp3");
  land = LoadSound("resources/sound/land.mp3");
  hit = LoadSound("resources/sound/hit.mp3");
}
void SoundEffect::Unload() {
  UnloadSound(lose);
  UnloadSound(jump);
  UnloadSound(land);
  UnloadSound(hit);
}

void SoundEffect::PlayLand() { PlaySound(land); }
void SoundEffect::PlayLose() { PlaySound(lose); }
void SoundEffect::PlayJump() { PlaySound(jump); }
void SoundEffect::PlayHit() { PlaySound(hit); }
