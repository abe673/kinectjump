#pragma once

#include "GameStates.h"
#include "raylib.h"

class CameraEffects {
public:
  void Reset();
  void Update(float dt, bool airborne);
  void Apply(Camera3D &camera) const;

  void TriggerJump();
  void TriggerLanding(bool perfectLanding, Direction arah);
  void TriggerHit();

private:
  float yaw = 0.0f;
  float yawVelocity = 0.0f;
  float yawTarget = 0.0f;
  float dolly = 0.0f;
  float dollyVelocity = 0.0f;
  float fovKick = 0.0f;
  float fovKickVelocity = 0.0f;
  float roll = 0.0f;
  float rollVelocity = 0.0f;
  float shakeTimer = 0.0f;
  float shakeDuration = 0.0f;
  float shakeStrength = 0.0f;
  float bounce = 0.0f;
  float bounceVelocity = 0.0f;
  float shakeTime = 0.0f;
};
