
#pragma once

#include "kinect_types.hpp"
#include <Windows.h>
#include <NuiApi.h>

#include <thread>
#include <mutex>
#include <atomic>

class KinectDevice {
public:
    KinectDevice();
    ~KinectDevice();

    bool start();
    void stop();

    // ambil snapshot frame terakhir
    bool getFrame(kinect::Frame& outFrame);

private:
    void sensorLoop();

    // =================================================
    // Kinect SDK
    // =================================================
    INuiSensor* sensor = nullptr;
    HANDLE depthStream = nullptr;
    HANDLE skeletonEvent = nullptr;

    // =================================================
    // Threading
    // =================================================
    std::thread worker;
    std::atomic<bool> running{ false };

    std::mutex frameMutex;

    // double buffer
    kinect::Frame frontFrame;
    kinect::Frame backFrame;
};
