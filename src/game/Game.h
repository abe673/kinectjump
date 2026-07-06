#pragma once
#include "../KinectStatus.h"
#include "CameraEffects.h"
#include "Character.h"
#include "GameStates.h"
#include "HUD.h"
#include "KinectInstruction.h"
#include "ParallaxBackground.h"
#include "PlatformManager.h"
#include "ScoreSystem.h"
#include "SpriteEffect.h"
#include "raymath.h"

struct CountDown {
  static constexpr float START_TIME = 3.f;
  static constexpr float GO_DURATION = 1.0f;
  float timer = START_TIME;
  int current = 3;
  int previous = 4;

  bool Update(float dt) {
    timer -= dt;
    current = (int)ceilf(timer);
    if (current != previous) {
      previous = current;
      return true;
    }
    return false;
  }
  bool IsOver() const { return timer <= -GO_DURATION; }
  void Reset() {
    timer = START_TIME;
    current = 3;
    previous = 4;
  }
  float Progress() const {
    if (timer >= 0.0f) {
      float frac = timer - floorf(timer);
      return 1.0f - frac;
    } else {
      // GO
      return (-timer) / GO_DURATION;
    }
  }
};

struct ResultState {
  static constexpr float DISPLAY_TIME = 2.2f;

  bool won = false;
  float timer = 0.0f;

  void Show(bool isWon) {
    won = isWon;
    timer = DISPLAY_TIME;
  }

  bool Update(float dt) {
    timer -= dt;
    return timer <= 0.0f;
  }

  void Reset() {
    won = false;
    timer = 0.0f;
  }
};

struct GameplayState {
  ScoreSystem systemScore;

  float survivalTimer = 20.0f;
  bool gameOver = false;

  void UpdateScore(bool isperfectlanded) {
    systemScore.AddLandingScore(isperfectlanded);
  }

  void Finish(bool won) {
    systemScore.finalscore.Show(won);
    gameOver = true;
    survivalTimer = 0.0f;
    if (won) {
      systemScore.AddSurvivalWinBonus();
    }
  }
  bool FinalTimerIsEnd(float dt) {
    systemScore.finalscore.timer -= dt;
    systemScore.finalscore.appear += dt;
    if (systemScore.finalscore.timer <= 0.0f) {
      return true;
    }
    return false;
  }

  void Reset() {
    systemScore.Reset();
    gameOver = false;
    survivalTimer = systemScore.SURVIVAL_TIME_LIMIT;
  }
};

class Game {
public:
  void Init();
  void Update(float dt);
  void Draw();
  void Shutdown();

  Camera3D camera{};
  Game(KinectStatus &_kinectStatus);

private:
  void HandleInput();
  void SetKinectStatus();
  void Reset();
  void UpdateWaitingForPlayer(float dt);
  void UpdateCountdown(float dt);
  void UpdatePlaying(float dt);
  void UpdateGameplay(float dt);
  void UpdateResult(float dt);
  void UpdateFinalScore(float dt);
  void HandleEvent(const PlatformEvent &event);
  void UpdateCamera();
  void DrawGameplayScene();
  void DrawSurvivalTimer();
  void DrawCountdown();
  void DrawResultOverlay();
  void DrawLandingFeedback();
  void DrawKinectInstructions();
  void DrawFinalScoreScene();
  void StartCountdown();
  void StartResult(bool won);
  void StartFinalScore(bool won);

private:
  KinectInstruction kinectInstruction;
  KinectStatus &statusKinect;

  GameState state = GameState::WaitingForPlayer;
  HUD hud;

  GameplayState playState;
  CountDown countdown;
  ResultState result;

  CameraEffects cameraEffects;
  ParallaxBackground parallax;

  Character fox;
  PlatformManager platforms;
  SpriteEffect stars;
  float cameraHeight = 0.0f;

  // bool kinectPlayerTracked = false;
  // bool kinectReadyPose = false;
  // float kinectReadyHoldTime = 0.0f;
  // float kinectReadyHoldRequired = 1.0f;

  // bool lastStatusKinnect = false;
  // float trackingLostTimer = 0.0f;
  //  bool waitingReconnect = false;

  Sound countsound;
  Sound goSound;
  Sound trackedsound;
  Sound notTrackedSound;
  Sound perfectsound;

  Model island;
  // Font poppins;
  //  Texture2D skyboxtex;
};
