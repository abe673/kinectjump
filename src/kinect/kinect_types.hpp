
#pragma once

#include <array>
#include <vector>
#include <cstdint>

namespace kinect {

// =====================================================
// CONSTANTS (REPLACE #define)
// =====================================================
constexpr int DepthWidth  = 320;
constexpr int DepthHeight = 240;
constexpr int MaxSkeletons = 6;
constexpr int JointCount   = 20;

// =====================================================
// ENUMS
// =====================================================
enum class JointType : uint8_t {
    HipCenter = 0,
    Spine,
    ShoulderCenter,
    Head,
    ShoulderLeft,
    ElbowLeft,
    WristLeft,
    HandLeft,
    ShoulderRight,
    ElbowRight,
    WristRight,
    HandRight,
    HipLeft,
    KneeLeft,
    AnkleLeft,
    FootLeft,
    HipRight,
    KneeRight,
    AnkleRight,
    FootRight,
    Count
};


using Bone = std::pair<JointType, JointType>;

static constexpr Bone Bones[] = {
    { JointType::Head,          JointType::ShoulderCenter },
    { JointType::ShoulderCenter,JointType::Spine },
    { JointType::Spine,         JointType::HipCenter },

    { JointType::ShoulderCenter,JointType::ShoulderLeft },
    { JointType::ShoulderLeft,  JointType::ElbowLeft },
    { JointType::ElbowLeft,     JointType::WristLeft },
    { JointType::WristLeft,     JointType::HandLeft },

    { JointType::ShoulderCenter,JointType::ShoulderRight },
    { JointType::ShoulderRight, JointType::ElbowRight },
    { JointType::ElbowRight,    JointType::WristRight },
    { JointType::WristRight,    JointType::HandRight },

    { JointType::HipCenter,     JointType::HipLeft },
    { JointType::HipLeft,       JointType::KneeLeft },
    { JointType::KneeLeft,      JointType::AnkleLeft },
    { JointType::AnkleLeft,     JointType::FootLeft },

    { JointType::HipCenter,     JointType::HipRight },
    { JointType::HipRight,      JointType::KneeRight },
    { JointType::KneeRight,     JointType::AnkleRight },
    { JointType::AnkleRight,    JointType::FootRight }
};

enum class TrackingState : uint8_t {
    NotTracked = 0,
    Inferred   = 1,
    Tracked    = 2
};

// =====================================================
// BASIC MATH TYPES
// =====================================================
struct Vec2f {
    float x = 0.0f;
    float y = 0.0f;
};

struct Vec3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// =====================================================
// JOINT
// =====================================================
struct Joint {
    Vec3f position;                 // world space (meters)
    Vec2f depthPosition;             // depth image space
    TrackingState state = TrackingState::NotTracked;
};

// =====================================================
// SKELETON
// =====================================================
struct Skeleton {
    uint32_t trackingId = 0;
    bool     tracked    = false;

    std::array<Joint, JointCount> joints;

    inline const Joint& get(JointType type) const {
        return joints[static_cast<size_t>(type)];
    }

    inline Joint& get(JointType type) {
        return joints[static_cast<size_t>(type)];
    }
};

// =====================================================
// DEPTH FRAME
// =====================================================
struct DepthFrame {
    int width  = DepthWidth;
    int height = DepthHeight;

    std::vector<uint16_t> data;
    uint64_t timestamp = 0;

    DepthFrame()
        : data(width * height, 0)
    {}

    inline uint16_t& at(int x, int y) {
        return data[y * width + x];
    }

    inline const uint16_t& at(int x, int y) const {
        return data[y * width + x];
    }
};

// =====================================================
// FRAME (SYNCED DATA)
// =====================================================
struct Frame {
    uint64_t timestamp = 0;

    bool hasDepth    = false;
    bool hasSkeleton = false;

    DepthFrame depth;
    std::array<Skeleton, MaxSkeletons> skeletons;
};

} // namespace kinect
