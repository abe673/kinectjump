#include "ParallaxBackground.h"

void ParallaxBackground::Init() {
  layers[0].texture = LoadTexture("resources/parallax/layer1.png");
  layers[0].parallax = 0.10f;
  layers[0].windSpeed = 8.0f;

  layers[1].texture = LoadTexture("resources/parallax/layer2.png");
  layers[1].parallax = 0.18f;
  layers[1].windSpeed = 12.0f;

  layers[2].texture = LoadTexture("resources/parallax/layer3.png");
  layers[2].parallax = 0.28f;
  layers[2].windSpeed = 18.0f;

  layers[3].texture = LoadTexture("resources/parallax/layer4.png");
  layers[3].parallax = 0.40f;
  layers[3].windSpeed = 25.0f;
}

void ParallaxBackground::Update(float dt) {
  for (auto &layer : layers) {
    layer.offsetX += layer.windSpeed * dt;

    while (layer.offsetX >= layer.texture.width)
      layer.offsetX -= layer.texture.width;
  }
}
constexpr float PIXELS_PER_UNIT = 150.0f;
void ParallaxBackground::Draw(float cameraY) const {
  const int screenWidth = GetScreenWidth();

  for (const auto &layer : layers) {

    float y = cameraY * PIXELS_PER_UNIT * layer.parallax;

    // mulai satu texture di luar layar kiri
    float startX = -layer.offsetX - layer.texture.width;

    // gambar sampai melewati sisi kanan layar
    for (float x = startX; x < screenWidth + layer.texture.width;
         x += layer.texture.width) {
      DrawTexture(layer.texture, (int)x, (int)y, WHITE);
    }
  }
  // DrawText(TextFormat("cameraY = %.2f", cameraY), 20, 20, 20, RED);
}

void ParallaxBackground::Unload() {
  for (auto &layer : layers) {
    UnloadTexture(layer.texture);
  }
}
