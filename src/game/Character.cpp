#include "Character.h"

#include "raymath.h"
#include "rlgl.h"

static constexpr float ANIM_FPS = 30.0f;
static constexpr Vector3 SCALE = {1.0f, 1.0f, 1.0f};
static constexpr float JUMP_SPEED = 7.2f;
static constexpr float GRAVITY = 18.0f;
static constexpr float FALL_LIMIT = -18.0f;
static constexpr float HIT_PUSH = 1.1f;

enum {
  ANIM_IDLE,
  ANIM_JUMP,
  ANIM_LOOK_LEFT,
  ANIM_LOOK_RIGHT,
  ANIM_DEATH,
  ANIM_WIN
};

void Character::Init() {
  model = LoadModel("resources/foxmodel.iqm");

  texture = LoadTexture("resources/jakqi.png");

  anims = LoadModelAnimations("resources/foxmodel.iqm", &animCount);

  model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = texture;

  sfx.Init();
  Reset(0.0f);
}
Character::~Character() {
  if (model.meshCount > 0)
    UnloadModel(model);

  if (texture.id > 0)
    UnloadTexture(texture);

  if (anims)
    UnloadModelAnimations(anims, animCount);

  sfx.Unload();
}

bool Character::Jump() {
  // allow jump when grounded OR within coyote time window
  if (state != CharacterState::Grounded && coyoteTimer <= 0.0f)
    return false;

  jumpStartY = position.y;

  state = CharacterState::Airborne;

  velocity.y = JUMP_SPEED;

  // consume coyote time
  coyoteTimer = 0.0f;

  BlendTo(ANIM_JUMP);
  sfx.PlayJump();
  return true;
}

void Character::Land(float y) {
  state = CharacterState::Grounded;

  position.y = y;

  velocity.y = 0.0f;

  groundY = y;
  jumpStartY = y;
  if (facingDirection == Direction::Left) {
    BlendTo(ANIM_LOOK_LEFT);
  } else {
    BlendTo(ANIM_LOOK_RIGHT);
  }
  sfx.PlayLand();
}

void Character::Face(Direction dir) {
  facingDirection = dir;

  targetFacingAngle = (dir == Direction::Left) ? 50.0f : -50.0f;
}

void Character::Die(Direction dir) { Die(dir, position.y); }

void Character::Die(Direction dir, float y) {
  state = CharacterState::Dead;

  position.y = y;

  velocity = Vector3Zero();

  blending = false;
  currentAnim = ANIM_DEATH;
  currentFrame = 40.0f;

  if (dir == Direction::Left) {
    fixedX = position.x + HIT_PUSH;
    targetFacingAngle = -90.0f;
  } else {
    fixedX = position.x - HIT_PUSH;
    targetFacingAngle = 90.0f;
  }
  facingAngle = targetFacingAngle;
  // TraceLog(LOG_WARNING, "anim=%d frame=%f", currentAnim, currentFrame);
  groundY = y;
  sfx.PlayHit();
}

void Character::Win() {
  state = CharacterState::Won;
  position.y = groundY;
  velocity = Vector3Zero();
  fixedX = position.x;
  targetFacingAngle = 0.0f;

  if (animCount > ANIM_WIN) {
    BlendTo(ANIM_WIN);
  } else {
    BlendTo(ANIM_IDLE);
  }
}

void Character::ApplyGravity(float dt) {
  float gravity = velocity.y > 0.0f ? GRAVITY : GRAVITY * 1.5f;

  velocity.y -= gravity * dt;

  if (velocity.y < FALL_LIMIT) {
    velocity.y = FALL_LIMIT;
  }
}

void Character::UpdatePhysics(float dt) {
  previousPosition = position;

  if (state == CharacterState::Dead) {
    if (currentFrame < 52.0f) {
      position.y = groundY;
      position.x = fixedX;
      return;
    }

    if (!deathSoundPlayed) {
      sfx.PlayLose();
      deathSoundPlayed = true;
    }
  }

  if (state == CharacterState::Grounded) {
    // while grounded we reset coyote timer so player has a small grace window
    position.y = groundY;
    velocity.y = 0.0f;
    coyoteTimer = COYOTE_TIME;
  } else if (state == CharacterState::Won) {
    position.y = groundY;
    velocity = Vector3Zero();
  } else {
    // airborne: decrement coyote timer
    if (coyoteTimer > 0.0f) {
      coyoteTimer -= dt;
      if (coyoteTimer < 0.0f) coyoteTimer = 0.0f;
    }

    ApplyGravity(dt);

    position.y += velocity.y * dt;
  }

  position.x = fixedX;
}

void Character::Update(float dt) {
  facingAngle = Lerp(facingAngle, targetFacingAngle, dt * 8.0f);

  UpdatePhysics(dt);

  UpdateAnimation(dt);
}

void Character::UpdateIdle(float dt) {
  if (currentAnim != ANIM_IDLE || blending) {
    currentAnim = ANIM_IDLE;
    currentFrame = 0.0f;
    blending = false;
  }

  facingAngle = Lerp(facingAngle, targetFacingAngle, dt * 8.0f);
  position.y = groundY;
  velocity = Vector3Zero();
  UpdateAnimation(dt);
}

void Character::Draw() {
  rlDisableBackfaceCulling();
  DrawModelEx(model, position, {0, 1, 0}, facingAngle, SCALE, WHITE);
  rlEnableBackfaceCulling();
}

bool Character::IsGrounded() const { return state == CharacterState::Grounded; }

bool Character::IsFalling() const { return velocity.y <= 0.0f; }

Vector3 Character::GetPosition() const { return position; }

Vector3 Character::GetPreviousPosition() const { return previousPosition; }

float Character::GetJumpStartY() const { return jumpStartY; }

void Character::UpdateAnimation(float dt) {
  currentFrame += ANIM_FPS * dt;

  float maxFrame = (float)(anims[currentAnim].keyframeCount - 1);

  if (currentAnim == ANIM_IDLE) {
    if (currentFrame >= maxFrame)
      currentFrame = 0.0f;
  } else {
    if (currentFrame >= maxFrame)
      currentFrame = maxFrame;
  }

  if (blending) {
    nextFrame += ANIM_FPS * dt;

    blendTimer += dt;

    float blend = Clamp(blendTimer / blendDuration, 0.0f, 1.0f);

    UpdateModelAnimationEx(model, anims[currentAnim], currentFrame,
                           anims[nextAnim], nextFrame, blend);

    if (blend >= 1.0f) {
      currentAnim = nextAnim;
      currentFrame = nextFrame;

      blending = false;
    }

    return;
  }

  UpdateModelAnimationEx(model, anims[currentAnim], currentFrame,
                         anims[currentAnim], currentFrame, 0.0f);
}
void Character::Reset(float y) {
  state = CharacterState::Grounded;

  position = {0.0f, y, 0.0f};

  previousPosition = position;

  velocity = Vector3Zero();

  groundY = y;
  jumpStartY = y;

  fixedX = 0.0f;

  facingAngle = 0.0f;
  targetFacingAngle = 0.0f;

  currentAnim = ANIM_IDLE;
  currentFrame = 0.0f;

  blending = false;
  nextAnim = -1;
  nextFrame = 0.0f;

  blendTimer = 0.0f;

  currentAnim = ANIM_IDLE;
  currentFrame = 0.0f;
  deathSoundPlayed = false;
}
void Character::BlendTo(int anim) {
  if (anim < 0 || anim >= animCount)
    return;

  if (anim == currentAnim && !blending)
    return;

  nextAnim = anim;
  nextFrame = 0.0f;

  blending = true;
  blendTimer = 0.0f;
}
