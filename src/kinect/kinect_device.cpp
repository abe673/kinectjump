
#include "kinect_device.hpp"
#include <cstring>

using namespace kinect;

// =====================================================
// Constructor / Destructor
// =====================================================
KinectDevice::KinectDevice() {}

KinectDevice::~KinectDevice() {
    stop();
}

// =====================================================
// Start / Stop
// =====================================================
bool KinectDevice::start() {
    int count = 0;
    if (FAILED(NuiGetSensorCount(&count)) || count == 0)
        return false;

    if (FAILED(NuiCreateSensorByIndex(0, &sensor)))
        return false;

    if (FAILED(sensor->NuiInitialize(
        NUI_INITIALIZE_FLAG_USES_DEPTH |
        NUI_INITIALIZE_FLAG_USES_SKELETON)))
        return false;

    if (FAILED(sensor->NuiImageStreamOpen(
        NUI_IMAGE_TYPE_DEPTH,
        NUI_IMAGE_RESOLUTION_320x240,
        0,
        2,
        nullptr,
        &depthStream)))
        return false;

    skeletonEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (!skeletonEvent)
        return false;

    if (FAILED(sensor->NuiSkeletonTrackingEnable(skeletonEvent, 0)))
        return false;

    running = true;
    worker = std::thread(&KinectDevice::sensorLoop, this);
    return true;
}

void KinectDevice::stop() {
    running = false;

    if (worker.joinable())
        worker.join();

    if (sensor) {
        sensor->NuiShutdown();
        sensor->Release();
        sensor = nullptr;
    }

    if (skeletonEvent) {
        CloseHandle(skeletonEvent);
        skeletonEvent = nullptr;
    }
}

// =====================================================
// Main Sensor Loop
// =====================================================
void KinectDevice::sensorLoop() {
    while (running && sensor) {

        bool updated = false;

        // ================= DEPTH =================
        NUI_IMAGE_FRAME frame;
        if (SUCCEEDED(sensor->NuiImageStreamGetNextFrame(
                depthStream, 0, &frame))) {

            INuiFrameTexture* tex = frame.pFrameTexture;
            NUI_LOCKED_RECT rect;
            tex->LockRect(0, &rect, nullptr, 0);

            if (rect.Pitch) {
                for (int y = 0; y < DepthHeight; ++y) {
                    std::memcpy(
                        &backFrame.depth.data[y * DepthWidth],
                        static_cast<uint8_t*>(rect.pBits) + y * rect.Pitch,
                        DepthWidth * sizeof(uint16_t)
                    );
                }

                backFrame.depth.timestamp = frame.liTimeStamp.QuadPart;
                backFrame.hasDepth = true;
                updated = true;
            }

            tex->UnlockRect(0);
            sensor->NuiImageStreamReleaseFrame(depthStream, &frame);
        }

        // ================= SKELETON =================
        if (WaitForSingleObject(skeletonEvent, 0) == WAIT_OBJECT_0) {
            NUI_SKELETON_FRAME skFrame;
            if (SUCCEEDED(sensor->NuiSkeletonGetNextFrame(0, &skFrame))) {

                backFrame.hasSkeleton = true;
                backFrame.timestamp = skFrame.liTimeStamp.QuadPart;

                for (int i = 0; i < MaxSkeletons; ++i) {
                    Skeleton& dst = backFrame.skeletons[i];
                    const NUI_SKELETON_DATA& src = skFrame.SkeletonData[i];

                    dst.trackingId = src.dwTrackingID;
                    dst.tracked = (src.eTrackingState == NUI_SKELETON_TRACKED);

                    if (!dst.tracked)
                        continue;

                    for (int j = 0; j < JointCount; ++j) {
                        const Vector4& p = src.SkeletonPositions[j];
                        Joint& joint = dst.joints[j];

                        LONG x, y;
                        USHORT depth;

                        NuiTransformSkeletonToDepthImage(
                            p,
                            &x,
                            &y,
                            &depth,
                            NUI_IMAGE_RESOLUTION_320x240
                        );

                        joint.position = { p.x, p.y, p.z };

                        joint.depthPosition = {
                            static_cast<float>(x),
                            static_cast<float>(y)
                        };

                        joint.state =
                            (src.eSkeletonPositionTrackingState[j] == NUI_SKELETON_POSITION_TRACKED)
                                ? TrackingState::Tracked
                                : (src.eSkeletonPositionTrackingState[j] == NUI_SKELETON_POSITION_INFERRED)
                                    ? TrackingState::Inferred
                                    : TrackingState::NotTracked;
                    }
                }

                updated = true;
            }
        }

        // ================= SWAP BUFFER =================
        if (updated) {
            std::lock_guard<std::mutex> lock(frameMutex);
            std::swap(frontFrame, backFrame);
        }

        Sleep(1);
    }
}

// =====================================================
// Public API
// =====================================================
bool KinectDevice::getFrame(Frame& outFrame) {
    std::lock_guard<std::mutex> lock(frameMutex);
    outFrame = frontFrame;
    return frontFrame.hasDepth || frontFrame.hasSkeleton;
}
