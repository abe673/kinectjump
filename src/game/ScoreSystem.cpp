#include "ScoreSystem.h"

namespace {
constexpr float COMBO_DISPLAY_TIME = 1.0f;
constexpr float PERFECT_DISPLAY_TIME = 0.85f;
constexpr int LANDING_SCORE = 25;
constexpr int PERFECT_SCORE = 100;
} // namespace

void ScoreSystem::Reset() {
  score = 0;
  lastScoreGain = 0;
  perfectComboCount = 0;
  comboTimer = 0.0f;
  perfectTimer = 0.0f;
  finalscore.Reset();
}

void ScoreSystem::Update(float dt) {
  if (comboTimer > 0.0f) {
    comboTimer -= dt;
  }
  if (perfectTimer > 0.0f) {
    perfectTimer -= dt;
  }
}

int ScoreSystem::AddLandingScore(bool perfectLanding) {
  lastScoreGain = LANDING_SCORE;

  if (perfectLanding) {
    perfectComboCount++;
    lastScoreGain += PERFECT_SCORE * perfectComboCount;
    comboTimer = COMBO_DISPLAY_TIME;
    perfectTimer = PERFECT_DISPLAY_TIME;
  } else {
    ResetCombo();
  }

  score += lastScoreGain;
  return lastScoreGain;
}

int ScoreSystem::AddSurvivalWinBonus() {
  lastScoreGain = SURVIVAL_WIN_BONUS;
  score += lastScoreGain;
  perfectTimer = PERFECT_DISPLAY_TIME;
  return lastScoreGain;
}

void ScoreSystem::ResetCombo() { perfectComboCount = 0; }

int ScoreSystem::GetScore() const { return score; }

int ScoreSystem::GetLastScoreGain() const { return lastScoreGain; }

int ScoreSystem::GetPerfectComboCount() const { return perfectComboCount; }

float ScoreSystem::GetComboTimer() const { return comboTimer; }

float ScoreSystem::GetPerfectTimer() const { return perfectTimer; }
