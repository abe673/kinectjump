#pragma once

class PersonTracker;

enum GestureType {
  GESTURE_JUMP = 0,
  GESTURE_READY,

  GESTURE_COUNT
};

enum JumpState { JUMP_STAND, JUMP_CROUCH, JUMP_RISING };

class KinectGesture {
public:
  void Update(const PersonTracker &person);

  bool IsGesturePressed(GestureType g) const;
  bool IsGestureReleased(GestureType g) const;
  bool IsGestureDown(GestureType g) const;

  bool IsJump() const;

  // pose
  bool IsReadyPose() const;
  int GetScore() const;

private:
  bool current[GESTURE_COUNT] = {};
  bool previous[GESTURE_COUNT] = {};

  JumpState jumpState = JUMP_STAND;

  float standingHeadY = 0.0f;
  float standingHipY = 0.0f;
  bool standingInitialized = false;
  int jumpCooldownFrames = 0;

  float standingFootLeftY = 0.0f;
  float standingFootRightY = 0.0f;
  bool standingFeetInitialized = false;

  float bodyHeight = 0.0f;
  int latestScore = 0;
};
