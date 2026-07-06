#pragma once

#include "raylib.h"
#include <cstdint>
#include <vector>
class SpriteEffect {
public:
  void Init(const char *file, int columns, int rows, float fps);

  void Spawn(Vector3 position, float size = 1.0f);

  void Update(float dt);

  void Draw(const Camera3D &camera);

  void Unload();
  ~SpriteEffect();

private:
  struct Instance {
    Vector3 position{};
    float timer = 0.0f;
    float size = 1.0f;
    bool active = true;
  };

  Texture2D texture{};

  std::vector<Instance> instances;

  int cols = 1;
  int rows = 1;
  int frameCount = 1;

  float animFPS = 30.0f;
};
