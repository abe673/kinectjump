#pragma once
#include "ScoreSystem.h"
#include "raylib.h"

class HUD {
public:
  HUD(ScoreSystem &_scoresystem, Camera &_camera);
  void Init();
  void Draw(Vector3 textWorldPos);
  void Unload();
  void DrawFinalScoreScene();
  void DrawCountdown(int _current, float dtime);
  void DrawTopHUD(float timerSurvive);

private:
  void DrawCenteredText(const char *text, Vector2 center, float size,
                        Color color);
  void DrawCombo();
  struct HUDLayout {
    // Position
    Vector2 scoreLabelPos;
    Vector2 scoreValuePos;

    Vector2 timerLabelPos;
    Vector2 timerValuePos;

    Vector2 dividerPos;

    // Font size
    float labelFontSize = 22.0f;
    float valueFontSize = 72.0f;

    // Shadow
    Vector2 shadowOffset = {3, 3};
    // combo
    Vector2 comboCenter;
    float comboFontSize = 30.0f;
  };

  HUDLayout layout;

  ScoreSystem &scoreSystem;
  Camera &camera;
  void DrawPerfectLanding(Vector2 loc);

private:
  Font poppins;
  int centerX;
  float centerY;
  int screenWidth;
  int screenHeight;
};
