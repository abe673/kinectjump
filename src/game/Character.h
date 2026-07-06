#pragma once

#include "GameStates.h"
#include "SoundEffect.h"
#include "raylib.h"
enum class CharacterState { Grounded, Airborne, Landing, Dead, Won };

class Character {
public:
  Character() = default;
  ~Character();

  void Init();

  void Update(float dt);
  void UpdateIdle(float dt);
  void Draw();

  bool Jump();

  void Land(float y);

  void Face(Direction dir);

  void Die(Direction dir);
  void Die(Direction dir, float y);
  void Win();

  void Reset(float y);

  bool IsGrounded() const;
  bool IsFalling() const;

  Vector3 GetPosition() const;
  Vector3 GetPreviousPosition() const;
  float GetJumpStartY() const;

public:
  Vector3 position{};

private:
  void UpdatePhysics(float dt);
  void UpdateAnimation(float dt);

  void ApplyGravity(float dt);

  void BlendTo(int anim);

  bool AnimationFinished() const;
  bool IsLoopAnimation(int anim) const;
  SoundEffect sfx;
  bool deathSoundPlayed = false;

private:
  CharacterState state = CharacterState::Grounded;
  Direction facingDirection = Direction::Left;
  Model model{};
  Texture2D texture{};
  // tail

  ModelAnimation *anims = nullptr;

  int animCount = 0;

  int currentAnim = 0;
  int nextAnim = -1;

  float currentFrame = 0.0f;
  float nextFrame = 0.0f;

  bool blending = false;

  float blendTimer = 0.0f;
  float blendDuration = 0.20f;

  Vector3 previousPosition{};
  Vector3 velocity{};

  float groundY = 0.0f;
  float jumpStartY = 0.0f;

  float facingAngle = 0.0f;
  float targetFacingAngle = 0.0f;

  float fixedX = 0.0f;
};
