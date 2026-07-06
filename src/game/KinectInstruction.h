#pragma once
#include "../KinectStatus.h"
#include "GameStates.h"
#include "raylib.h"
class KinectInstruction {
public:
  KinectInstruction(KinectStatus &status);
  void Draw(GameState state, bool gameOver);
  void Init();

private:
  void UpdateMessage(GameState state, bool gameOver);
  void DrawPanel() const;
  void DrawMessage() const;
  void DrawProgress() const;

private:
  const int INSTRUCTION_FONT_SIZE = 22;
  KinectStatus &status;

  // layout
  int screenWidth;
  int screenHeight;
  int boxWidth;
  int boxHeight;
  int x;
  int y;

  int barX;
  int barY;
  int barWidth;

  // render state
  const char *title = "";
  const char *line1 = "";
  const char *line2 = "";

  Color accent = WHITE;
  bool showProgress = false;
  float progress = 0.f;
};
