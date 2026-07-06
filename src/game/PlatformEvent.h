#pragma once

#include "GameStates.h"

enum class PlatformEventType {
  None,
  Landed,
  Hit,
  JumpedTooEarly,
  PlatformPassed
};

struct PlatformEvent {
  PlatformEventType type = PlatformEventType::None;

  Direction direction = Direction::Left;

  float landingY = 0.0f;

  bool perfectLanding = false;
};
