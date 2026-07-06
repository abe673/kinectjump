#include "CameraEffects.h"

#include "raymath.h"

#include <cmath>

namespace {
constexpr float CAMERA_SPRING = 10.0f;
constexpr float CAMERA_DAMPING = 10.0f;

constexpr float SHAKE_SPEED = 32.0f;

// Jump
constexpr float JUMP_DOLLY = 0.18f;
constexpr float JUMP_ROLL = 0.4f;
constexpr float HIT_DOLLY = 3.0f;

// Landing
constexpr float LANDING_BOUNCE = 0.08f;
constexpr float LANDING_FOV = -0.15f;
constexpr float LANDING_SHAKE = 0.04f;

// Hit
constexpr float HIT_SHAKE = 0.10f;
constexpr float HIT_ROLL = 3.0f;
constexpr float HIT_FOV = 2.0f;
constexpr float HIT_YAW = 1.0f;

float Spring(float current, float &velocity, float target, float dt) {
  float force = (target - current) * CAMERA_SPRING;
  velocity += force * dt;
  velocity -= velocity * CAMERA_DAMPING * dt;
  return current + velocity * dt;
}
} // namespace

void CameraEffects::Reset() {
  dolly = 0.0f;
  dollyVelocity = 0.0f;
  fovKick = 0.0f;
  fovKickVelocity = 0.0f;
  roll = 0.0f;
  rollVelocity = 0.0f;
  shakeTimer = 0.0f;
  shakeDuration = 0.0f;
  shakeStrength = 0.0f;
  bounce = 0.0f;
  bounceVelocity = 0.0f;
  shakeTime = 0.0f;
  yaw = 0.0f;
  yawVelocity = 0.0f;
  yawTarget = 0.0f;
}

void CameraEffects::Update(float dt, bool airborne) {
  yaw = Spring(yaw, yawVelocity, yawTarget, dt);
  // Target saat di udara
  float targetDolly = airborne ? JUMP_DOLLY : 0.0f;

  dolly = Spring(dolly, dollyVelocity, targetDolly, dt);

  // Bounce selalu kembali ke nol
  bounce = Spring(bounce, bounceVelocity, 0.0f, dt);

  // FOV
  fovKick = Spring(fovKick, fovKickVelocity, 0.0f, dt);

  // Roll
  roll = Spring(roll, rollVelocity, 0.0f, dt);

  if (shakeTimer > 0.0f) {
    shakeTimer -= dt;
    shakeTime += dt;

    if (shakeTimer <= 0.0f) {
      shakeTimer = 0.0f;
      shakeDuration = 0.0f;
      shakeStrength = 0.0f;
      shakeTime = 0.0f;
    }
  }
}

void CameraEffects::Apply(Camera3D &camera) const {
  Vector3 forward =
      Vector3Normalize(Vector3Subtract(camera.target, camera.position));

  Matrix rot = MatrixRotateY(yaw * DEG2RAD);

  forward = Vector3Transform(forward, rot);

  camera.target = Vector3Add(camera.position, forward);

  float fade = 0.0f;

  if (shakeTimer > 0.0f && shakeDuration > 0.0f)
    fade = shakeTimer / shakeDuration;

  float shakeX = sinf(shakeTime * SHAKE_SPEED) * shakeStrength * fade;

  float shakeY =
      cosf(shakeTime * SHAKE_SPEED * 1.37f) * shakeStrength * 0.5f * fade;

  camera.position.x += shakeX;
  camera.position.y += shakeY + bounce;
  camera.position.z += dolly;

  camera.target.x += shakeX * 0.35f;
  camera.target.y += shakeY * 0.35f + bounce * 0.25f;

  camera.fovy += fovKick;

  float rollRadians = roll * DEG2RAD;
  camera.up = {sinf(rollRadians), cosf(rollRadians), 0.0f};
}
void CameraEffects::TriggerJump() {
  dollyVelocity += 3.5f;
  rollVelocity += 2.0f;
}

void CameraEffects::TriggerLanding(bool perfectLanding, Direction arah) {
  yawTarget = (arah == Direction::Left) ? 4.0f : -4.0f;

  bounceVelocity -=
      perfectLanding ? LANDING_BOUNCE * 7.0f : LANDING_BOUNCE * 5.0f;

  dollyVelocity -= perfectLanding ? 8.0f : 4.0f; // 2x lipat lebih kuat
  shakeTimer = perfectLanding ? 0.18f : 0.12f;
  shakeDuration = shakeTimer;
  shakeStrength = perfectLanding ? LANDING_SHAKE : LANDING_SHAKE * 0.6f;

  rollVelocity -= perfectLanding ? JUMP_ROLL * 3.0f : JUMP_ROLL * 2.0f;

  fovKickVelocity += perfectLanding ? LANDING_FOV * 2.0f : LANDING_FOV;
}

void CameraEffects::TriggerHit() {
  dollyVelocity -= HIT_DOLLY;
  bounceVelocity += 1.5f;

  shakeTimer = 0.30f;
  shakeDuration = shakeTimer;
  shakeStrength = HIT_SHAKE;

  rollVelocity += HIT_ROLL * 2.0f;

  fovKickVelocity += HIT_FOV * 2.0f;
}
