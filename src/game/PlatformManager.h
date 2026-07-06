#pragma once

#include "GameStates.h"
#include "PlatformEvent.h"

#include "raylib.h"

struct Platform {
  Vector3 pos{};
  Vector3 previousPos{};
  Direction side = Direction::Left;
  bool active = false;
  float speed = 0.0f;
  int modelId = 0;
};

class PlatformManager {
public:
  ~PlatformManager();
  void Init();
  void Reset();
  PlatformEvent Update(float dt, Vector3 previousPlayerPos, Vector3 playerPos,
                       bool playerFalling, bool playerGrounded);
  void Draw();
  float GetHeight() const;
  float GetPlayerStartY() const;

private:
  void SpawnPlatform();
  void UpdateSpawnTimer(float dt);
  void MovePlatform(float dt);
  bool CheckLanding(Vector3 previousPlayerPos, Vector3 currentPlayerPos,
                    bool playerFalling, PlatformEvent &result);
  bool CheckPlayerCrossing(Vector3 playerPos, bool playerGrounded,
                           PlatformEvent &result);
  void PushPlatform(Vector3 pos);
  bool CheckPlatformPassed(PlatformEvent &event);

private:
  const float startPlatformSpeed = 4.0f;
  float currentPlatformSpeed = startPlatformSpeed;
  Platform activePlatform{};
  Platform platforms[10];
  int platformCount = 0;
  float worldHeight = 0.0f;
  float nextPlatformY = 0.0f;
  float spawnDelay = 0.0f;
  Direction nextSpawnSide = Direction::Left;
  float platformHalfWidth = 1.0f;
  float platformTopOffset = 0.0f;
  float platformHeight = 0.0f;
  // Model model{};
  Model kayu;
  int nextidmodel = 0;
};
