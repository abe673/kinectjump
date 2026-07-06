#pragma once
struct KinectStatus {
  const float READY_HOLD_REQUIRED = 1.0f;
  const float TRACKING_LOST_LIMIT = 10.0f;

  bool tracked = false;
  bool readyPose = false;
  float holdTime = 0.f;
  float requiredHold = 1.f;
  bool jump = false;
  bool startTriggered = false;
  bool waitingReconnect = false;
  bool lastStatusKinnect = false;
  float trackingLostTimer = 0.0f;
  void Reset() {
    trackingLostTimer = 0.0f;
    waitingReconnect = false;
  }
  void Update(bool isTracked, bool isReadyPose, bool isJump, float dt) {
    tracked = isTracked;
    readyPose = isReadyPose;
    jump = isJump;

    startTriggered = false;

    if (tracked && readyPose) {
      holdTime += dt;

      if (holdTime >= requiredHold) {
        startTriggered = true;
      }
    } else {
      holdTime = 0.f;
      startTriggered = false;
    }
  }
};
