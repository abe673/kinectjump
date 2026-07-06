
#pragma once
#include "kinect_types.hpp"

#include "raylib.h"

struct PersonSlot {
  uint32_t trackingId = 0;
  bool active = false;
};

static PersonSlot gPersons[kinect::MaxSkeletons];

inline void UpdatePersons(const kinect::Frame &frame) {
  // reset active flag
  for (auto &p : gPersons)
    p.active = false;

  if (!frame.hasSkeleton) {
    for (auto &p : gPersons)
      p.trackingId = 0;
    return;
  }

  // assign skeleton → slot
  for (const auto &sk : frame.skeletons) {
    if (!sk.tracked)
      continue;

    bool found = false;

    // 1. sudah ada?
    for (auto &p : gPersons) {
      if (p.trackingId == sk.trackingId) {
        p.active = true;
        found = true;
        break;
      }
    }

    // 2. orang baru → slot kosong
    if (!found) {
      for (auto &p : gPersons) {
        if (p.trackingId == 0) {
          p.trackingId = sk.trackingId;
          p.active = true;
          break;
        }
      }
    }
  }

  // 3. buang orang yang hilang
  for (auto &p : gPersons) {
    if (!p.active)
      p.trackingId = 0;
  }
}

template <typename Fn>
bool WithPerson(const kinect::Frame &frame, int personIndex, Fn &&fn) {
  if (personIndex < 0 || personIndex >= kinect::MaxSkeletons)
    return false;

  uint32_t id = gPersons[personIndex].trackingId;
  if (id == 0)
    return false;

  for (const auto &sk : frame.skeletons) {
    if (sk.tracked && sk.trackingId == id) {
      fn(sk);
      return true;
    }
  }
  return false;
}

class DepthRenderer {
public:
  DepthRenderer();
  ~DepthRenderer();
  void Init();

  void Update(const kinect::Frame &frame);
  void Draw(int x, int y, float scale);
  void DrawSkeletons(const kinect::Frame &frame, int x, int y, float scale);

private:
  // ---- GPU ----
  Texture2D depthTex;
  Shader grayShader;
  Image image;
  bool initialized = false;
};
