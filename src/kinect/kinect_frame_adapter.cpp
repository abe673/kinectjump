
#include "kinect_frame_adapter.hpp"

using namespace kinect;

// =====================================================
// Constructor
// =====================================================
KinectFrameAdapter::KinectFrameAdapter(KinectDevice& dev)
    : device(dev)
{
    frame.hasDepth = false;
    frame.hasSkeleton = false;
}

// =====================================================
// Update
// =====================================================
bool KinectFrameAdapter::update() {
    // ambil snapshot frame dari device
    if (!device.getFrame(frame))
        return false;

    // ================= SKELETON SELECTION =================
    // Pilih skeleton pertama yang tracked (mirip perilaku lama)
    if (frame.hasSkeleton) {
        bool found = false;

        for (auto& sk : frame.skeletons) {
            if (sk.tracked) {
                found = true;
                break;
            }
        }

        frame.hasSkeleton = found;
    }

    return frame.hasDepth || frame.hasSkeleton;
}

// =====================================================
// Getters
// =====================================================
const Frame& KinectFrameAdapter::getFrame() const {
    return frame;
}

const Skeleton* KinectFrameAdapter::getPrimarySkeleton() const {
    if (!frame.hasSkeleton)
        return nullptr;

    for (const auto& sk : frame.skeletons) {
        if (sk.tracked)
            return &sk;
    }
    return nullptr;
}
