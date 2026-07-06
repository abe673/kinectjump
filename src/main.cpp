
#include "RaylibApp.h"
#include "kinect/kinect_device.hpp"
#include "kinect/kinect_frame_adapter.hpp"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
  RaylibApp app(1080, 1920, "Kinect Sandbox");
  KinectDevice device;
  if (!device.start()) {
    MessageBoxA(nullptr, "Kinect gagal start", "Error", MB_ICONERROR);
    return -1;
  }

  KinectFrameAdapter adapter(device);

  while (!app.ShouldClose()) {

    adapter.update();
    const kinect::Frame &frame = adapter.getFrame();
    app.Update(frame);
  }
  return 0;
}
