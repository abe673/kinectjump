#pragma once
#include "../kinect/kinect_types.hpp"

struct TargetPerson {
  uint32_t trackingId = 0;
  float lostTime = 0.0f;

  bool HasTarget() const { return trackingId != 0; }

  void Clear() {
    trackingId = 0;
    lostTime = 0.0f;
  }
};

struct ActivationZone {
  float minX = -0.5f;
  float maxX = 0.5f;

  float minZ = 1.7f;
  // center 2.30
  float maxZ = 2.8f;

  float CenterX() const { return (minX + maxX) * 0.5f; }
  float CenterZ() const { return (minZ + maxZ) * 0.5f; }
};

struct RingBuffer {
  float data[10] = {};
  int head = 0;
  int count = 0;

  void push(float v) {
    data[head] = v;
    head = (head + 1) % 10;
    if (count < 10)
      count++;
  }
  void Clear() {
    head = 0;
    count = 0;
  }
  float get(int i) const {
    if (i < 0 || i >= count)
      return 0.0f;
    int idx = (head - count + i + 10) % 10;
    return data[idx];
  }

  float latest() const {
    if (count == 0)
      return 0.0f;

    return get(count - 1);
  }
};

struct BoneDataXYZ {
  RingBuffer x;
  RingBuffer y;
  RingBuffer z;
  void Add(kinect::Vec3f val) {
    x.push(val.x);
    y.push(val.y);
    z.push(val.z);
  }
  void Clear() {
    x.Clear();
    y.Clear();
    z.Clear();
  }
};

class PersonTracker {
public:
  BoneDataXYZ handR;
  BoneDataXYZ handL;
  BoneDataXYZ head;
  BoneDataXYZ shoulderCenter;
  BoneDataXYZ hipCenter;
  BoneDataXYZ footLeft;
  BoneDataXYZ footRight;

  void Update(const kinect::Frame &frame);
  void Reset();

  bool HasTarget() const { return target.trackingId != 0; }

  uint32_t TargetId() const { return target.trackingId; }

  TargetPerson target;
  bool IsTracked() const;
  bool HasLeftHand() const { return handLTracked; }
  bool HasRightHand() const { return handRTracked; }
  bool HasLeftFoot() const { return footLeftTracked; }
  bool HasRightFoot() const { return footRightTracked; }
  const kinect::Vec3f &GetHipPosition() const;

private:
  static constexpr int TARGET_ACQUIRE_FRAMES = 5;
  static constexpr int TRACKING_GRACE_FRAMES = 8;
  static constexpr int TARGET_LOST_FRAMES = 36;
  static constexpr float EXIT_ZONE_MARGIN = 0.15f;

  kinect::Vec3f hipPosition{};
  bool tracked = false;
  int lostFrames = 0;
  uint32_t candidateId = 0;
  int candidateFrames = 0;
  bool handLTracked = false;
  bool handRTracked = false;
  bool footLeftTracked = false;
  bool footRightTracked = false;
  ActivationZone zone;
  bool IsInActivationZone(const kinect::Vec3f &p, float margin = 0.0f) const;
  float DistanceFromZoneCenter(const kinect::Vec3f &p) const;
  bool HasReliableCoreJoints(const kinect::Skeleton &sk) const;
  void ClearCandidate();

  const kinect::Skeleton *FindBestCandidate(const kinect::Frame &frame) const;
  const kinect::Skeleton *FindTargetSkeleton(const kinect::Frame &frame) const;

  void UpdateBones(const kinect::Skeleton &sk);
};
