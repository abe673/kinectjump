#include "HUD.h"
#include "raymath.h"
#include <cstdio>
inline float EaseOutCubic(float t) { return 1.0f - powf(1.0f - t, 3.0f); }

inline float EaseOutBack(float t) {
  float c1 = 1.70158f;
  float c3 = c1 + 1.0f;

  return 1.0f + c3 * powf(t - 1.0f, 3) + c1 * powf(t - 1.0f, 2);
}
HUD::HUD(ScoreSystem &_scoreSystem, Camera &cam)
    : scoreSystem(_scoreSystem), camera(cam) {}
void HUD::Init() {
  screenWidth = GetScreenWidth();
  screenHeight = GetScreenHeight();
  centerX = screenWidth / 2;
  centerY = screenHeight * 0.5f;
  poppins = LoadFontEx("resources/poppinsbold.ttf", 96, nullptr, 0);
  SetTextureFilter(poppins.texture, TEXTURE_FILTER_POINT);

  constexpr float top = 20.0f;
  constexpr float valueOffset = 22.0f;
  constexpr float spacing = 120.0f;

  layout.scoreLabelPos = {centerX - spacing, top};
  layout.scoreValuePos = {centerX - spacing, top + valueOffset};

  layout.timerLabelPos = {centerX + spacing, top};
  layout.timerValuePos = {centerX + spacing, top + valueOffset};

  layout.dividerPos = {static_cast<float>(centerX), top + 6};
  layout.comboCenter = {static_cast<float>(centerX),
                        layout.scoreValuePos.y + 70.0f};
}

void HUD::Unload() { UnloadFont(poppins); }

void HUD::DrawCountdown(int current, float dtime) {
  float t = Clamp(1.0f - dtime, 0.0f, 1.0f);

  float scale = 1.0f;
  float alpha = 1.0f;

  if (t < 0.25f) {
    float p = EaseOutBack(t / 0.25f);

    scale = Lerp(2.3f, 1.0f, p);
    alpha = p;
  } else if (t < 0.75f) {
    scale = 1.0f;
    alpha = 1.0f;
  } else {
    float p = EaseOutCubic((t - 0.75f) / 0.25f);

    scale = Lerp(1.0f, 0.8f, p);
    alpha = 1.0f - p;
  }

  const char *text = current > 0 ? TextFormat("%d", current) : "GO!";

  constexpr float fontSize = 280.0f;

  Vector2 size = MeasureTextEx(poppins, text, fontSize, 0);

  Vector2 origin = {size.x * scale * 0.5f, size.y * scale * 0.5f};

  Vector2 pos = {(float)screenWidth / 2, (float)screenHeight / 2};

  Color shadow = Fade(BLACK, alpha * 0.35f);

  DrawTextPro(poppins, text, {pos.x + 6, pos.y + 6}, origin, 0,
              fontSize * scale, 0, shadow);

  DrawTextPro(poppins, text, pos, origin, 0, fontSize * scale, 0,
              Fade(WHITE, alpha));
}
void HUD::DrawCenteredText(const char *text, Vector2 center, float size,
                           Color color) {
  Vector2 textSize = MeasureTextEx(poppins, text, size, 1);

  Vector2 pos = {center.x - textSize.x * 0.5f, center.y};

  DrawTextEx(poppins, text, Vector2Add(pos, layout.shadowOffset), size, 1,
             Fade(BLACK, 0.35f));

  DrawTextEx(poppins, text, pos, size, 1, color);
}
void HUD::DrawTopHUD(float timerSurvive) {
  Color timerColor = WHITE;

  if (timerSurvive <= 5.0f) {
    timerColor = ((int)(GetTime() * 6) % 2 == 0) ? RED : ORANGE;
  }

  DrawCenteredText("SCORE", layout.scoreLabelPos, layout.labelFontSize,
                   Fade(WHITE, 0.7f));

  DrawCenteredText(TextFormat("%d", scoreSystem.GetScore()),
                   layout.scoreValuePos, layout.valueFontSize, WHITE);

  DrawCenteredText("TIME", layout.timerLabelPos, layout.labelFontSize,
                   Fade(WHITE, 0.7f));

  DrawCenteredText(TextFormat("%02d", (int)ceilf(timerSurvive)),
                   layout.timerValuePos, layout.valueFontSize, timerColor);

  DrawRectangle(layout.dividerPos.x - 1, layout.dividerPos.y, 2, 70,
                Fade(WHITE, 0.2f));
}

void HUD::DrawCombo() {
  if (scoreSystem.GetComboTimer() <= 0.0f)
    return;

  if (scoreSystem.GetPerfectComboCount() <= 1)
    return;

  const char *comboText =
      TextFormat("PERFECT COMBO x%d", scoreSystem.GetPerfectComboCount());

  float t = 1.0f - scoreSystem.GetComboTimer() / scoreSystem.COMBO_DISPLAY_TIME;

  float pop = EaseOutBack(Clamp(t * 4.0f, 0.0f, 1.0f));
  float alpha = 1.0f - t;

  float size = layout.comboFontSize * (0.7f + pop * 0.3f);

  DrawCenteredText(comboText, layout.comboCenter, size, Fade(ORANGE, alpha));
}

void HUD::Draw(Vector3 textWorldPos) {
  DrawCombo();

  if (scoreSystem.GetPerfectTimer() <= 0.0f) {
    return;
  }

  float alpha =
      Clamp(scoreSystem.GetPerfectTimer() / scoreSystem.PERFECT_DISPLAY_TIME,
            0.0f, 1.0f);

  textWorldPos.y += 1.05f;
  Vector2 textPos = GetWorldToScreen(textWorldPos, camera);
  const char *perfectText =
      TextFormat("PERFECT LANDING +%d", scoreSystem.GetLastScoreGain());

  float t =
      1.0f - scoreSystem.GetPerfectTimer() / scoreSystem.PERFECT_DISPLAY_TIME;
  float move = EaseOutCubic(t);
  int fontSize = (int)(28 * (1.0f + (1.0f - move) * 0.4f));
  int textWidth = MeasureText(perfectText, fontSize);

  DrawText(perfectText, (int)(textPos.x - textWidth * 0.5f), (int)textPos.y,
           fontSize, Fade(GOLD, alpha));
}

void HUD::DrawFinalScoreScene() {

  DrawRectangle(0, 0, screenWidth, screenHeight, {9, 13, 24, 255});

  DrawCircleGradient({(float)centerX, centerY}, (float)screenWidth * 0.62f,
                     Fade(BLUE, 0.28f), Fade(BLACK, 0.0f));

  DrawCircleGradient({(float)centerX, screenHeight / 3.0f},
                     (float)screenWidth * 0.38f, Fade(GOLD, 0.20f),
                     Fade(BLACK, 0.0f));

  float t = Clamp(scoreSystem.finalscore.appear / 0.6f, 0.0f, 1.0f);
  float panelScale = EaseOutBack(t);
  float alpha = t;

  const char *title = scoreSystem.finalscore.won ? "KAMU MENANG" : "SKOR AKHIR";

  int titleSize = (int)(46 * panelScale);
  int scoreSize = (int)(96 * panelScale);

  float titleY = centerY - 170.0f * panelScale;
  float scoreY = centerY - 95.0f * panelScale;
  float comboY = centerY + 25.0f * panelScale;
  float timerY = centerY + 78.0f * panelScale;
  float barY = centerY + 128.0f * panelScale;

  DrawText(title, centerX - MeasureText(title, titleSize) / 2, (int)titleY,
           titleSize, Fade(scoreSystem.finalscore.won ? GREEN : GOLD, alpha));

  const char *scoreText = TextFormat("%d", scoreSystem.GetScore());

  DrawText(scoreText, centerX - MeasureText(scoreText, scoreSize) / 2,
           (int)scoreY, scoreSize, Fade(WHITE, alpha));

  const char *comboText =
      scoreSystem.finalscore.won
          ? TextFormat("Berhasil bertahan %.0f detik! Bonus +%d",
                       scoreSystem.SURVIVAL_TIME_LIMIT,
                       scoreSystem.SURVIVAL_WIN_BONUS)
      : scoreSystem.GetPerfectComboCount() > 1
          ? TextFormat("Perfect combo terakhir x%d",
                       scoreSystem.GetPerfectComboCount())
          : "Coba lagi untuk perfect landing berikutnya";

  int comboSize = 24;

  DrawText(comboText, centerX - MeasureText(comboText, comboSize) / 2,
           (int)comboY, comboSize, Fade(WHITE, 0.86f * alpha));

  int secondsLeft = (int)ceilf(scoreSystem.finalscore.timer);

  const char *timerText =
      TextFormat("Kembali ke mode Kinect dalam %d detik", secondsLeft);

  int timerSize = 22;
  DrawText(timerText, centerX - MeasureText(timerText, timerSize) / 2,
           (int)timerY, timerSize, Fade(WHITE, 0.70f * alpha));

  const int barWidth = screenWidth < 560 ? screenWidth - 80 : 480;

  const int barX = centerX - barWidth / 2;

  float progress =
      Clamp(scoreSystem.finalscore.timer / scoreSystem.finalscore.DISPLAY_TIME,
            0.0f, 1.0f);

  DrawRectangle(barX, (int)barY, barWidth, 8, Fade(WHITE, 0.18f * alpha));

  DrawRectangle(barX, (int)barY, (int)(barWidth * progress), 8,
                Fade(GOLD, alpha));
}
