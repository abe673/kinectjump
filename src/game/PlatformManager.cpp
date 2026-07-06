#include "PlatformManager.h"

#include "raymath.h"

static constexpr float SPAWN_X = 6.5f;

static constexpr float PLAYER_HALF_WIDTH = 0.48f; // increased for easier landings

static constexpr float LANDING_SKIN = 0.35f; // more forgiving vertical tolerance

static constexpr float HIT_X_RANGE = 0.35f; // slightly increased

static constexpr float HIT_FOOT_GRACE = 0.18f; // more forgiving foot grace

static constexpr float MIN_SUPPORT = 0.30f; // reduced required support to make landings easier

static constexpr float SPAWN_DELAY = 0.35f; // longer delay between platforms

static constexpr float PERFECT_LANDING_X_RANGE = 0.30f;

void PlatformManager::Init() {
  kayu = LoadModel("resources/wood.glb");

  BoundingBox box = GetModelBoundingBox(kayu);
  platformHalfWidth = fmaxf(fabsf(box.min.x), fabsf(box.max.x));
  platformTopOffset = box.max.y;
  platformHeight = box.max.y - box.min.y;
  Reset();
}

void PlatformManager::Reset() {
  platformCount = 0;
  nextPlatformY = 0.0f;
  spawnDelay = 0.0f;
  worldHeight = platformTopOffset;
  PushPlatform({0.0f, nextPlatformY, 0.0f});
  nextPlatformY += platformHeight;
  nextSpawnSide =
      (GetRandomValue(0, 1) == 0) ? Direction::Left : Direction::Right;
  currentPlatformSpeed = startPlatformSpeed;
  SpawnPlatform();
}

void PlatformManager::SpawnPlatform() {
  float startX = nextSpawnSide == Direction::Left ? -SPAWN_X : SPAWN_X;

  activePlatform.pos = {startX, nextPlatformY, 0.0f};

  activePlatform.previousPos = activePlatform.pos;

  activePlatform.side = nextSpawnSide;

  activePlatform.active = true;

  activePlatform.speed = currentPlatformSpeed;
  activePlatform.modelId = nextidmodel;
}

void PlatformManager::MovePlatform(float dt) {
  activePlatform.previousPos = activePlatform.pos;

  float dir = activePlatform.side == Direction::Left ? 1.0f : -1.0f;

  activePlatform.pos.x += dir * activePlatform.speed * dt;
}

bool PlatformManager::CheckLanding(Vector3 previousPlayerPos,
                                   Vector3 currentPlayerPos, bool playerFalling,
                                   PlatformEvent &event) {
  float top = activePlatform.pos.y + platformTopOffset;

  float sweepLeft = fminf(activePlatform.previousPos.x, activePlatform.pos.x);

  float sweepRight = fmaxf(activePlatform.previousPos.x, activePlatform.pos.x);

  float left = sweepLeft - platformHalfWidth;
  float right = sweepRight + platformHalfWidth;

  float playerLeft = currentPlayerPos.x - PLAYER_HALF_WIDTH;

  float playerRight = currentPlayerPos.x + PLAYER_HALF_WIDTH;

  float support =
      fmaxf(0.0f, fminf(right, playerRight) - fmaxf(left, playerLeft));

  bool feetSupported = support >= PLAYER_HALF_WIDTH * 2.0f * MIN_SUPPORT;

  bool crossedTop = previousPlayerPos.y >= top - LANDING_SKIN &&
                    currentPlayerPos.y <= top + LANDING_SKIN;

  if (!playerFalling || !feetSupported || !crossedTop) {
    return false;
  }

  event.type = PlatformEventType::Landed;

  event.landingY = top;
  event.perfectLanding = fabsf(activePlatform.pos.x) <= PERFECT_LANDING_X_RANGE;

  worldHeight = top;

  activePlatform.active = false;

  PushPlatform(activePlatform.pos);

  nextPlatformY = activePlatform.pos.y + platformHeight;

  nextSpawnSide = GetRandomValue(0, 1) ? Direction::Right : Direction::Left;
  nextidmodel = GetRandomValue(0, 2);
  event.direction = nextSpawnSide;
  spawnDelay = SPAWN_DELAY;
  currentPlatformSpeed += 0.12f; // slower difficulty ramp
  return true;
}

bool PlatformManager::CheckPlayerCrossing(Vector3 playerPos,
                                          bool playerGrounded,
                                          PlatformEvent &event) {
  float top = activePlatform.pos.y + platformTopOffset;

  bool reachedPlayer = false;

  if (activePlatform.side == Direction::Left) {
    reachedPlayer = activePlatform.previousPos.x < -HIT_X_RANGE &&
                    activePlatform.pos.x >= -HIT_X_RANGE;
  } else {
    reachedPlayer = activePlatform.previousPos.x > HIT_X_RANGE &&
                    activePlatform.pos.x <= HIT_X_RANGE;
  }

  bool playerLow = playerPos.y < top - HIT_FOOT_GRACE;

  if (!reachedPlayer || !playerLow) {
    return false;
  }

  event.type = playerGrounded ? PlatformEventType::Hit
                              : PlatformEventType::JumpedTooEarly;

  event.direction = activePlatform.side;

  return true;
}

PlatformEvent PlatformManager::Update(float dt, Vector3 previousPlayerPos,
                                      Vector3 playerPos, bool playerFalling,
                                      bool playerGrounded) {
  PlatformEvent event{};
  UpdateSpawnTimer(dt);
  if (!activePlatform.active)
    return event;

  MovePlatform(dt);

  if (CheckLanding(previousPlayerPos, playerPos, playerFalling, event)) {
    return event;
  }

  if (CheckPlayerCrossing(playerPos, playerGrounded, event)) {
    return event;
  }

  if (CheckPlatformPassed(event)) {
    return event;
  }
  return event;
}
bool PlatformManager::CheckPlatformPassed(PlatformEvent &event) {
  if (!activePlatform.active)
    return false;

  bool passedPlayer = false;

  if (activePlatform.side == Direction::Left) {
    passedPlayer = activePlatform.pos.x > PLAYER_HALF_WIDTH + platformHalfWidth;
  } else {
    passedPlayer =
        activePlatform.pos.x < -(PLAYER_HALF_WIDTH + platformHalfWidth);
  }

  if (!passedPlayer)
    return false;

  event.type = PlatformEventType::PlatformPassed;
  event.direction = activePlatform.side;

  return true;
}
float PlatformManager::GetPlayerStartY() const { return worldHeight; }

float PlatformManager::GetHeight() const { return worldHeight; }

void PlatformManager::UpdateSpawnTimer(float dt) {
  if (spawnDelay <= 0.0f)
    return;

  spawnDelay -= dt;

  if (spawnDelay <= 0.0f) {
    SpawnPlatform();
  }
}
void PlatformManager::PushPlatform(Vector3 pos) {
  if (platformCount < 10) {
    platforms[platformCount].pos = pos;
    platforms[platformCount].previousPos = pos;
    platforms[platformCount].active = false;
    platforms[platformCount].modelId = activePlatform.modelId; // tambah ini
    platformCount++;

    return;
  }

  for (int i = 1; i < 10; i++) {
    platforms[i - 1] = platforms[i];
  }

  platforms[9].pos = pos;
  platforms[9].previousPos = pos;
  platforms[9].active = false;
  platforms[9].modelId = activePlatform.modelId; // tambah ini
}
static float getKayuAngle(int id) {
  switch (id) {
  case 1:
    return 90.0f;
    break;
  case 2:
    return 180.0f;
    break;
  default:
    return 0;
    break;
  }
}
void PlatformManager::Draw() {
  for (int i = 0; i < platformCount; i++) {
    DrawModelEx(kayu, platforms[i].pos, {0, 1, 0},
                getKayuAngle(platforms[i].modelId), Vector3One(), WHITE);
  }

  if (activePlatform.active) {
    DrawModelEx(kayu, activePlatform.pos, {0, 1, 0},
                getKayuAngle(activePlatform.modelId), Vector3One(), WHITE);
  }
}

PlatformManager::~PlatformManager() { UnloadModel(kayu); }
