#include "PersonTracker.h"
#include "../kinect/DepthRenderer.h"
#include "../kinect/kinect_types.hpp"
#include <cmath>
#include <iostream>
#include <limits>

void PersonTracker::UpdateBones(const kinect::Skeleton &sk) {
  handLTracked = false;
  handRTracked = false;
  footLeftTracked = false;
  footRightTracked = false;

  const auto &temp_head = sk.get(kinect::JointType::Head);

  const auto &temp_handR = sk.get(kinect::JointType::HandRight);

  const auto &temp_handL = sk.get(kinect::JointType::HandLeft);

  const auto &temp_shoulder = sk.get(kinect::JointType::ShoulderCenter);

  const auto &temp_hip = sk.get(kinect::JointType::HipCenter);

  const auto &temp_footR = sk.get(kinect::JointType::FootRight);

  const auto &temp_footL = sk.get(kinect::JointType::FootLeft);

  if (temp_head.state == kinect::TrackingState::Tracked) {
    head.Add(temp_head.position);
  }

  if (temp_handR.state == kinect::TrackingState::Tracked) {
    handR.Add(temp_handR.position);
    handRTracked = true;
  }

  if (temp_handL.state == kinect::TrackingState::Tracked) {
    handL.Add(temp_handL.position);
    handLTracked = true;
  }

  if (temp_shoulder.state == kinect::TrackingState::Tracked) {
    shoulderCenter.Add(temp_shoulder.position);
  }

  if (temp_hip.state == kinect::TrackingState::Tracked) {
    hipCenter.Add(temp_hip.position);
    hipPosition = temp_hip.position;
  }

  if (temp_footR.state == kinect::TrackingState::Tracked) {
    footRight.Add(temp_footR.position);
    footRightTracked = true;
  }

  if (temp_footL.state == kinect::TrackingState::Tracked) {
    footLeft.Add(temp_footL.position);
    footLeftTracked = true;
  }

  tracked = true;
}

const kinect::Skeleton *
PersonTracker::FindTargetSkeleton(const kinect::Frame &frame) const {
  if (target.trackingId == 0)
    return nullptr;

  for (const auto &sk : frame.skeletons) {
    if (!sk.tracked)
      continue;

    if (sk.trackingId == target.trackingId)
      return &sk;
  }

  return nullptr;
}

const kinect::Skeleton *
PersonTracker::FindBestCandidate(const kinect::Frame &frame) const {
  const kinect::Skeleton *best = nullptr;
  float bestDistance = std::numeric_limits<float>::max();

  for (const auto &sk : frame.skeletons) {
    if (!sk.tracked || !HasReliableCoreJoints(sk))
      continue;

    const auto &hip = sk.get(kinect::JointType::HipCenter);

    if (!IsInActivationZone(hip.position))
      continue;

    float distance = DistanceFromZoneCenter(hip.position);
    if (distance < bestDistance) {
      bestDistance = distance;
      best = &sk;
    }
  }

  return best;
}

bool PersonTracker::IsTracked() const { return tracked; }
void PersonTracker::Update(const kinect::Frame &frame) {
  tracked = false;
  handLTracked = false;
  handRTracked = false;
  footLeftTracked = false;
  footRightTracked = false;

  if (target.trackingId == 0) {
    const auto *candidate = FindBestCandidate(frame);

    if (!candidate) {
      ClearCandidate();
      return;
    }

    if (candidateId != candidate->trackingId) {
      candidateId = candidate->trackingId;
      candidateFrames = 1;
      return;
    }

    candidateFrames++;
    if (candidateFrames < TARGET_ACQUIRE_FRAMES)
      return;

    target.trackingId = candidate->trackingId;
    lostFrames = 0;
    ClearCandidate();
    UpdateBones(*candidate);
    return;
  }

  // sudah punya target
  auto *sk = FindTargetSkeleton(frame);

  if (!sk) {
    lostFrames++;
    tracked = lostFrames <= TRACKING_GRACE_FRAMES;
    if (lostFrames > TARGET_LOST_FRAMES)
      Reset();
    return;
  }

  const auto &hip = sk->get(kinect::JointType::HipCenter);

  if (!HasReliableCoreJoints(*sk) ||
      !IsInActivationZone(hip.position, EXIT_ZONE_MARGIN)) {
    lostFrames++;
    tracked = lostFrames <= TRACKING_GRACE_FRAMES;
    if (lostFrames > TARGET_LOST_FRAMES)
      Reset();
    return;
  }

  lostFrames = 0;
  UpdateBones(*sk);
}

bool PersonTracker::IsInActivationZone(const kinect::Vec3f &p,
                                       float margin) const {
  if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) {
    return false;
  }

  return p.x >= zone.minX - margin && p.x <= zone.maxX + margin &&
         p.z >= zone.minZ - margin && p.z <= zone.maxZ + margin;
}

float PersonTracker::DistanceFromZoneCenter(const kinect::Vec3f &p) const {
  float dx = p.x - zone.CenterX();
  float dz = p.z - zone.CenterZ();
  return dx * dx + dz * dz;
}

bool PersonTracker::HasReliableCoreJoints(const kinect::Skeleton &sk) const {
  const auto &head = sk.get(kinect::JointType::Head);
  const auto &shoulder = sk.get(kinect::JointType::ShoulderCenter);
  const auto &hip = sk.get(kinect::JointType::HipCenter);

  return head.state == kinect::TrackingState::Tracked &&
         shoulder.state == kinect::TrackingState::Tracked &&
         hip.state == kinect::TrackingState::Tracked;
}

const kinect::Vec3f &PersonTracker::GetHipPosition() const {
  return hipPosition;
}

void PersonTracker::ClearCandidate() {
  candidateId = 0;
  candidateFrames = 0;
}

void PersonTracker::Reset() {
  tracked = false;
  target.Clear();
  lostFrames = 0;
  ClearCandidate();
  handLTracked = false;
  handRTracked = false;
  footLeftTracked = false;
  footRightTracked = false;
  hipPosition = {};
  head.Clear();
  handR.Clear();
  handL.Clear();
  footLeft.Clear();
  footRight.Clear();
  shoulderCenter.Clear();
  hipCenter.Clear();
}
