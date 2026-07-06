#pragma once

#include "raylib.h"
#include <array>

class ParallaxBackground {
public:
  void Init();
  void Update(float dt);
  void Draw(float cameraY) const;
  void Unload();

private:
  struct Layer {
    Texture2D texture{};

    float parallax = 0.0f;
    float windSpeed = 0.0f;

    float offsetX = 0.0f;
  };

  static constexpr int LAYER_COUNT = 4;

  std::array<Layer, LAYER_COUNT> layers;
};
