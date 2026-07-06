#pragma once

struct FinalScoreState {
  bool won = false;
  float timer = 0.0f;
  float appear = 0.0f;

  static constexpr float DISPLAY_TIME = 5.0f;

  void Show(bool isWon) {
    won = isWon;
    timer = DISPLAY_TIME;
    appear = 0.0f;
  }

  void Reset() {
    won = false;
    timer = 0.0f;
    appear = 0.0f;
  }
};

class ScoreSystem {
public:
  void Reset();
  void Update(float dt);

  int AddLandingScore(bool perfectLanding);
  int AddSurvivalWinBonus();
  void ResetCombo();

  int GetScore() const;
  int GetLastScoreGain() const;
  int GetPerfectComboCount() const;
  float GetComboTimer() const;
  float GetPerfectTimer() const;

  const float COMBO_DISPLAY_TIME = 1.8f;
  const float PERFECT_DISPLAY_TIME = 1.8f;
  const int SURVIVAL_WIN_BONUS = 500;
  const float SURVIVAL_TIME_LIMIT = 20.0f;

  FinalScoreState finalscore;

private:
  int score = 0;
  int lastScoreGain = 0;
  int perfectComboCount = 0;
  float comboTimer = 0.0f;
  float perfectTimer = 0.0f;
};
