
#pragma once

#include "kinect_device.hpp"

// Adapter high-level (pengganti PrimitiveAdapter)
class KinectFrameAdapter {
public:
    explicit KinectFrameAdapter(KinectDevice& device);

    // update internal frame (non-blocking)
    bool update();

    // ambil frame hasil adaptasi
    const kinect::Frame& getFrame() const;

    // ambil skeleton utama (jika ada)
    const kinect::Skeleton* getPrimarySkeleton() const;

private:
    KinectDevice& device;
    kinect::Frame frame;
};
