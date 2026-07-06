#include "SpriteEffect.h"

void SpriteEffect::Init(const char *file, int columns, int rows_, float fps) {
  Unload();

  texture = LoadTexture(file);

  cols = columns;
  rows = rows_;
  frameCount = cols * rows;
  animFPS = fps;
}

SpriteEffect::~SpriteEffect() { Unload(); }
void SpriteEffect::Unload() {
  if (texture.id != 0) {
    UnloadTexture(texture);
    texture = {};
  }

  instances.clear();
}
void SpriteEffect::Spawn(Vector3 position, float size) {
  Instance e;

  e.position = position;
  e.size = size;

  instances.push_back(e);
}

void SpriteEffect::Update(float dt) {
  for (auto &e : instances) {
    if (!e.active)
      continue;

    e.timer += dt;

    int frame = (int)(e.timer * animFPS);

    if (frame >= frameCount)
      e.active = false;
  }
}

void SpriteEffect::Draw(const Camera3D &camera) {

  float frameWidth = (float)texture.width / cols;

  float frameHeight = (float)texture.height / rows;

  for (auto &e : instances) {

    if (!e.active)
      continue;

    int frame = (int)(e.timer * animFPS);

    if (frame >= frameCount)
      continue;

    int col = frame % cols;
    int row = frame / cols;

    Rectangle src{col * frameWidth, row * frameHeight, frameWidth, frameHeight};
    Vector2 Pos = GetWorldToScreen(e.position, camera);
    DrawTextureRec(texture, src, {Pos.x - 40, Pos.y - 90}, WHITE);
  }
}
