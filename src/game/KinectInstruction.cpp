#include "KinectInstruction.h"
#include "raymath.h"

static void DrawFittedText(const char *text, int x, int y, int maxWidth,
                           int fontSize, Color color) {
  int fittedSize = fontSize;
  while (fittedSize > 14 && MeasureText(text, fittedSize) > maxWidth) {
    fittedSize--;
  }
  DrawText(text, x, y, fittedSize, color);
}

inline float EaseOutCubic(float t) { return 1.0f - powf(1.0f - t, 3.0f); }

inline float EaseOutBack(float t) {
  float c1 = 1.70158f;
  float c3 = c1 + 1.0f;

  return 1.0f + c3 * powf(t - 1.0f, 3) + c1 * powf(t - 1.0f, 2);
}

KinectInstruction::KinectInstruction(KinectStatus &_kinectstatus)
    : status(_kinectstatus) {}

void KinectInstruction::Init() {
  screenWidth = GetScreenWidth();
  screenHeight = GetScreenHeight();

  boxWidth = screenWidth < 620 ? screenWidth - 32 : 560;
  boxHeight = 132;

  x = (screenWidth - boxWidth) / 2;
  y = screenHeight - boxHeight - 24;

  barX = x + 24;
  barY = y + boxHeight - 18;
  barWidth = boxWidth - 48;
}
void KinectInstruction::Draw(GameState state, bool gameOver) {
  UpdateMessage(state, gameOver);

  DrawPanel();
  DrawMessage();

  if (showProgress)
    DrawProgress();
}
void KinectInstruction::UpdateMessage(GameState state, bool gameOver) {
  title = "KINECT MODE";
  line1 = "";
  line2 = "";
  accent = SKYBLUE;

  showProgress = false;
  progress = 0.f;

  if (!status.tracked) {
    title = "PEMAIN BELUM TERDETEKSI";
    line1 = "Berdiri di tengah area media Trampolin.";
    line2 = "Permainan akan segera dimulai..";
    accent = ORANGE;
  } else if (state == GameState::WaitingForPlayer) {
    title = "PEMAIN TERDETEKSI";
    line1 = status.readyPose ? "Tahan kedua tangan di atas bahu..."
                             : "Angkat kedua tangan di atas bahu untuk siap.";

    line2 = "Setelah countdown, lompat sungguhan untuk melompati platform.";

    accent = status.readyPose ? GREEN : SKYBLUE;

    showProgress = true;
    progress = Clamp(status.holdTime / status.READY_HOLD_REQUIRED, 0.f, 1.f);
  } else if (state == GameState::Countdown) {
    title = "BERSIAP";
    line1 = "Turunkan tangan, fokus ke platform berikutnya.";
    line2 = "Lompat saat karakter perlu melewati rintangan.";
    accent = GREEN;
  } else {
    title = "MAIN";
    line1 = "Bertahan 30 detik untuk menang.";
    line2 = "Lompat sungguhan dan mendarat tepat untuk skor lebih besar.";
  }

  if (status.waitingReconnect) {
    title = "MENCARI PEMAIN";
    line1 = "Kembali ke area sensor untuk melanjutkan.";
    line2 = TextFormat("Waktu tunggu: %.0f detik",
                       status.TRACKING_LOST_LIMIT - status.trackingLostTimer);

    accent = RED;
    showProgress = true;

    progress = Clamp((status.TRACKING_LOST_LIMIT - status.trackingLostTimer) /
                         status.TRACKING_LOST_LIMIT,
                     0.f, 1.f);
  }

  if (gameOver) {
    showProgress = true;
    progress = Clamp(status.holdTime / status.READY_HOLD_REQUIRED, 0.f, 1.f);
  }
}
void KinectInstruction::DrawPanel() const {
  DrawRectangleRounded({(float)x, (float)y, (float)boxWidth, (float)boxHeight},
                       0.08f, 8, Fade(BLACK, 0.68f));

  DrawRectangleRoundedLines(
      {(float)x, (float)y, (float)boxWidth, (float)boxHeight}, 0.08f, 8,
      Fade(accent, 0.85f));
}
void KinectInstruction::DrawMessage() const {
  const int textMaxWidth = boxWidth - 48;

  DrawFittedText(title, x + 24, y + 18, textMaxWidth, 24, accent);

  DrawFittedText(line1, x + 24, y + 54, textMaxWidth, INSTRUCTION_FONT_SIZE,
                 WHITE);

  DrawFittedText(line2, x + 24, y + 84, textMaxWidth, INSTRUCTION_FONT_SIZE,
                 Fade(WHITE, 0.85f));
}
void KinectInstruction::DrawProgress() const {
  DrawRectangle(barX, barY, barWidth, 6, Fade(WHITE, 0.18f));

  DrawRectangle(barX, barY, (int)(barWidth * progress), 6, accent);
}
