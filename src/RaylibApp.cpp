#include "RaylibApp.h"
#include "KinectStatus.h"
#include "game/Game.h"
#include "gesture/KinectGesture.h"
#include "kinect/DepthRenderer.h"
#include "raylib.h"
#include "tracking/PersonTracker.h"

DepthRenderer depthView;
PersonTracker trackPerson;
KinectGesture gestur;
float readyTimer = 0.0f;
KinectStatus statuskinect;

Game game(statuskinect);
Color bgcolor = {21, 30, 49, 1};

RaylibApp::RaylibApp(int width, int height, const char *title)
    : width(width), height(height) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(width, height, title);
  InitAudioDevice();
  SetTargetFPS(30);
  ToggleFullscreen();
  DisableCursor();
  depthView.Init();
  game.Init();
}

RaylibApp::~RaylibApp() {
  game.Shutdown();
  CloseWindow();
}

bool RaylibApp::ShouldClose() { return WindowShouldClose(); }
bool showdebug = true;
bool showtextdebug = false;
bool triggered = false;
void RaylibApp::Update(const kinect::Frame &frame) {
  float dt = GetFrameTime();

  depthView.Update(frame);
  trackPerson.Update(frame);
  gestur.Update(trackPerson);

  statuskinect.Update(trackPerson.IsTracked(), gestur.IsReadyPose(),
                      gestur.IsJump(), dt);

  game.Update(dt);

  if (IsKeyPressed(KEY_D))
    showdebug = !showdebug;
  if (IsKeyPressed(KEY_T)) {
    showtextdebug = !showtextdebug;
  }
  BeginDrawing();
  ClearBackground(bgcolor);

  game.Draw();

  if (showdebug) {
    depthView.Draw(0, 0, 1); // tampil 640x480
    depthView.DrawSkeletons(frame, 0, 0, 1);
    if (showtextdebug) {

      DrawFPS(10, 10);
      DrawText(TextFormat("rise %d", gestur.GetScore()), 60, 30, 30, GREEN);
      DrawText(TextFormat("z = %0.2f", trackPerson.GetHipPosition().z), 40, 60,
               30, GREEN);
      DrawText(TextFormat("target %u tracked %d", trackPerson.TargetId(),
                          trackPerson.IsTracked()),
               40, 90, 30, GREEN);
      DrawText(
          TextFormat("ready %d jump %d", gestur.IsReadyPose(), gestur.IsJump()),
          40, 120, 30, GREEN);
      DrawText(TextFormat("hip x %0.2f y %0.2f", trackPerson.GetHipPosition().x,
                          trackPerson.GetHipPosition().y),
               40, 160, 30, GREEN);
    }
  }
  EndDrawing();
}
