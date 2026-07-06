
#pragma once
#include "kinect/kinect_types.hpp"
#include <cstdint>

class RaylibApp {
public:
  RaylibApp(int width, int height, const char *title);
  ~RaylibApp();
  bool ShouldClose();
  void Update(const kinect::Frame &frame);

private:
  int width;
  int height;
};
