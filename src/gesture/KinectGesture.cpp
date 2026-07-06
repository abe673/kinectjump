#include "KinectGesture.h"
#include "../tracking/PersonTracker.h"
#include <cstring>
#include <cmath>

constexpr float READY_HAND_GRACE = 0.05f;
constexpr float CROUCH_DROP = -0.060f;

constexpr float QUICK_HEAD_VELOCITY = 0.070f;
constexpr float QUICK_HIP_VELOCITY = 0.050f;
constexpr float QUICK_FOOT_VELOCITY = 0.035f;

constexpr float MIN_HEAD_RISE = 0.120f;
constexpr float MIN_HIP_RISE = 0.110f;

constexpr float MIN_HEAD_VELOCITY = 0.095f;
constexpr float MIN_HIP_VELOCITY = 0.095f;

constexpr float MIN_FOOT_RISE = 0.050f;
constexpr float MIN_FOOT_VELOCITY = 0.025f;

constexpr float RESET_RISE = 0.015f;

// lower cooldown to make detection more responsive while using weighted scoring
constexpr int JUMP_COOLDOWN_FRAMES = 6;

// anti-bounce thresholds
constexpr float BOUNCE_STDDEV_THRESHOLD = 0.03f; // tuneable
constexpr float BOUNCE_FOOT_RISE_MULT = 1.0f;    // foot rise compared to minFootRise

static float Delta(const RingBuffer &b, int frames) {
  if (b.count < frames)
    return 0.0f;

  return b.latest() - b.get(b.count - frames);
}

static float SmoothLatest(const RingBuffer &b) {
  if (b.count == 0)
    return 0.0f;

  int samples = b.count < 3 ? b.count : 3;
  float total = 0.0f;
  for (int i = b.count - samples; i < b.count; i++) {
    total += b.get(i);
  }
  return total / (float)samples;
}

static float ClampFloat(float value, float minValue, float maxValue) {
  if (value < minValue)
    return minValue;
  if (value > maxValue)
    return maxValue;
  return value;
}

void KinectGesture::Update(const PersonTracker &p) {
  memcpy(previous, current, sizeof(current));
  memset(current, 0, sizeof(current));

  //----------------------------------------
  // TRACK LOST
  //----------------------------------------

  if (!p.IsTracked()) {
    standingInitialized = false;
    standingFeetInitialized = false;
    jumpState = JUMP_STAND;
    jumpCooldownFrames = 0;
    return;
  }

  //----------------------------------------
  // READY POSE
  //----------------------------------------

  float shoulderY = p.shoulderCenter.y.latest();

  bool handsTracked = p.HasRightHand() && p.HasLeftHand();
  bool ready = handsTracked &&
               p.handR.y.latest() > shoulderY - READY_HAND_GRACE &&
               p.handL.y.latest() > shoulderY - READY_HAND_GRACE;

  current[GESTURE_READY] = ready;

  //----------------------------------------
  // CALIBRATION
  //----------------------------------------

  float headY = SmoothLatest(p.head.y);
  float hipY = SmoothLatest(p.hipCenter.y);
  bool feetTracked = p.HasLeftFoot() && p.HasRightFoot();
  float footLeftY =
      feetTracked ? SmoothLatest(p.footLeft.y) : standingFootLeftY;
  float footRightY =
      feetTracked ? SmoothLatest(p.footRight.y) : standingFootRightY;
  float leftFootRise = feetTracked ? footLeftY - standingFootLeftY : 0.0f;
  float rightFootRise = feetTracked ? footRightY - standingFootRightY : 0.0f;
  float footRise = feetTracked ? (leftFootRise + rightFootRise) * 0.5f : 0.0f;

  if (ready) {
    if (!standingInitialized) {
      standingHeadY = headY;
      standingHipY = hipY;
      standingFootLeftY = footLeftY;
      standingFootRightY = footRightY;
      standingFeetInitialized = feetTracked;
      bodyHeight = feetTracked
                       ? standingHeadY -
                             ((standingFootLeftY + standingFootRightY) * 0.5f)
                       : (standingHeadY - standingHipY) * 2.2f;
      standingInitialized = true;
    }

    // smooth calibration
    standingHeadY = standingHeadY * 0.94f + headY * 0.06f;
    standingHipY = standingHipY * 0.94f + hipY * 0.06f;
    if (feetTracked) {
      if (!standingFeetInitialized) {
        standingFootLeftY = footLeftY;
        standingFootRightY = footRightY;
        standingFeetInitialized = true;
      } else {
        standingFootLeftY = standingFootLeftY * 0.94f + footLeftY * 0.06f;
        standingFootRightY = standingFootRightY * 0.94f + footRightY * 0.06f;
      }
      bodyHeight =
          standingHeadY - ((standingFootLeftY + standingFootRightY) * 0.5f);
    } else {
      bodyHeight = (standingHeadY - standingHipY) * 2.2f;
    }

    jumpState = JUMP_STAND;
    jumpCooldownFrames = 0;
  }

  if (!standingInitialized)
    return;

  //----------------------------------------
  // JUMP
  //----------------------------------------

  if (jumpCooldownFrames > 0) {
    jumpCooldownFrames--;
  }

  float headRise = headY - standingHeadY;
  float hipRise = hipY - standingHipY;
  float headVelocity = Delta(p.head.y, 2);
  float hipVelocity = Delta(p.hipCenter.y, 2);
  bool feetReady = feetTracked && standingFeetInitialized;
  float leftFootVelocity = feetReady ? Delta(p.footLeft.y, 2) : 0.0f;
  float rightFootVelocity = feetReady ? Delta(p.footRight.y, 2) : 0.0f;

  float footVelocity =
      feetReady ? (leftFootVelocity + rightFootVelocity) * 0.5f : 0.0f;

  // tighten fastUpwardPush to rely on head+hip velocity to avoid foot-only bounces
  bool fastUpwardPush = hipVelocity > QUICK_HIP_VELOCITY &&
                        headVelocity > QUICK_HEAD_VELOCITY;

  float scale = ClampFloat(bodyHeight / 1.70f, 0.75f, 1.25f);

  float minHeadRise = MIN_HEAD_RISE * scale;
  float minHipRise = MIN_HIP_RISE * scale;
  float minFootRise = MIN_FOOT_RISE * scale;
  float crouchDrop = CROUCH_DROP * scale;
  float resetRise = RESET_RISE * scale;
  float crouchRecover = -0.01f * scale;

  bool bodyDipped = headRise < crouchDrop || hipRise < crouchDrop;

  // weighted scoring to make detection responsive but robust against foot-only bounces
  const float W_HIP_RISE = 1.2f;
  const float W_HIP_VEL = 1.0f;
  const float W_HEAD_RISE = 1.2f;
  const float W_HEAD_VEL = 1.0f;
  const float W_FOOT_RISE = 0.45f;
  const float W_FOOT_VEL = 0.45f;

  float weightedScore = 0.0f;
  if (hipRise > minHipRise)
    weightedScore += W_HIP_RISE;
  if (hipVelocity > MIN_HIP_VELOCITY)
    weightedScore += W_HIP_VEL;
  if (headRise > minHeadRise)
    weightedScore += W_HEAD_RISE;
  if (headVelocity > MIN_HEAD_VELOCITY)
    weightedScore += W_HEAD_VEL;
  if (feetReady && footRise > minFootRise)
    weightedScore += W_FOOT_RISE;
  if (feetReady && footVelocity > MIN_FOOT_VELOCITY)
    weightedScore += W_FOOT_VEL;

  if (weightedScore != 0.0f)
    latestScore = (int)roundf(weightedScore);
  bool confirmedRise = (weightedScore >= 3.0f);

  // ANTI-BOUNCE: detect cases where head/hip/feet rise uniformly (likely trampoline bounce)
  float nHead = headRise / (bodyHeight + 1e-6f);
  float nHip = hipRise / (bodyHeight + 1e-6f);
  float nFoot = footRise / (bodyHeight + 1e-6f);
  float mean = (nHead + nHip + nFoot) / 3.0f;
  float v1 = (nHead - mean) * (nHead - mean);
  float v2 = (nHip - mean) * (nHip - mean);
  float v3 = (nFoot - mean) * (nFoot - mean);
  float stddev = sqrtf((v1 + v2 + v3) / 3.0f);

  bool likelyBounce = (stddev <= BOUNCE_STDDEV_THRESHOLD) &&
                      (feetReady && footRise >= minFootRise * BOUNCE_FOOT_RISE_MULT) &&
                      (headVelocity < MIN_HEAD_VELOCITY || hipVelocity < MIN_HIP_VELOCITY) &&
                      (jumpState != JUMP_CROUCH);

  // require either a prior crouch OR strong combined head+hip movement OR confirmed weighted rise
  bool baseCandidate = !ready && jumpCooldownFrames == 0 &&
                       ((jumpState == JUMP_CROUCH && (fastUpwardPush || confirmedRise)) ||
                        (fastUpwardPush && headRise > minHeadRise && hipRise > minHipRise && feetReady) ||
                        (confirmedRise && feetReady));

  // if likely bounce and player didn't crouch, ignore candidate
  bool jumpCandidate = baseCandidate && !likelyBounce;

  switch (jumpState) {
  case JUMP_STAND:

    if (bodyDipped) {
      jumpState = JUMP_CROUCH;
      break;
    }

    if (jumpCandidate) {
      current[GESTURE_JUMP] = true;
      jumpState = JUMP_RISING;
      jumpCooldownFrames = JUMP_COOLDOWN_FRAMES;
    }

    break;

  case JUMP_CROUCH:

    if (jumpCandidate ||
        (headVelocity > MIN_HEAD_VELOCITY && hipVelocity > MIN_HIP_VELOCITY &&
         headRise > crouchRecover)) {
      current[GESTURE_JUMP] = true;
      jumpState = JUMP_RISING;
      jumpCooldownFrames = JUMP_COOLDOWN_FRAMES;
    } else if (headRise > crouchRecover && hipRise > crouchRecover) {
      jumpState = JUMP_STAND;
    }

    break;

  case JUMP_RISING:

    // tunggu kembali mendekati posisi normal
    if (headRise < resetRise && hipRise < resetRise &&
        headVelocity <= MIN_HEAD_VELOCITY)
      jumpState = JUMP_STAND;

    break;

  default:
    jumpState = JUMP_STAND;
    break;
  }
}
int KinectGesture::GetScore() const { return latestScore; }
bool KinectGesture::IsGesturePressed(GestureType g) const {
  return current[g] && !previous[g];
}

bool KinectGesture::IsGestureReleased(GestureType g) const {
  return !current[g] && previous[g];
}

bool KinectGesture::IsGestureDown(GestureType g) const { return current[g]; }

bool KinectGesture::IsJump() const { return IsGesturePressed(GESTURE_JUMP); }

bool KinectGesture::IsReadyPose() const { return IsGestureDown(GESTURE_READY); }
