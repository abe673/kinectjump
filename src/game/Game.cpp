#include "Game.h"
#include "raymath.h"

constexpr float CAMERA_DISTANCE = 11.0f;
constexpr float CAMERA_HEIGHT_OFFSET = 0.0f;
constexpr float CAMERA_TARGET_OFFSET = 1.0f;
constexpr float CAMERA_FOLLOW_SPEED = 0.08f;

Game::Game(KinectStatus &_kinectstatus)
    : statusKinect(_kinectstatus), kinectInstruction(_kinectstatus),
      hud(playState.systemScore, camera) {}

void Game::Init() {
  camera.position = {0.0f, 0.0f, CAMERA_DISTANCE};
  camera.target = {0.0f, 0.0f, 0.0f};
  camera.up = {0, 1, 0};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;
  platforms.Init();
  fox.Init();
  stars.Init("resources/stars.png", 4, 2, 20.0f);
  parallax.Init();
  countsound = LoadSound("resources/sound/count.mp3");
  goSound = LoadSound("resources/sound/go.mp3");
  notTrackedSound = LoadSound("resources/sound/nottrackedsound.mp3");
  trackedsound = LoadSound("resources/sound/trackedsound.mp3");
  perfectsound = LoadSound("resources/sound/perfectsound.mp3");

  island = LoadModel("resources/floating_island.glb");
  // poppins = LoadFont("resources/poppinsbold.ttf");
  kinectInstruction.Init();
  hud.Init();
  Reset();
}

void Game::Reset() {
  platforms.Reset();

  fox.Reset(platforms.GetPlayerStartY());
  cameraEffects.Reset();
  cameraHeight = platforms.GetHeight();

  camera.position = {0.0f, cameraHeight + CAMERA_HEIGHT_OFFSET,
                     CAMERA_DISTANCE};

  camera.target = {0.0f, cameraHeight + CAMERA_TARGET_OFFSET, 0.0f};

  state = GameState::WaitingForPlayer;
  countdown.Reset();
  result.Reset();
  playState.Reset();
  statusKinect.Reset();
}

void Game::StartCountdown() {
  state = GameState::Countdown;
  countdown.Reset();
}

void Game::StartResult(bool won) {
  state = GameState::Result;
  result.Show(won);
  playState.gameOver = true;
  if (won) {
    fox.Win();
  }
}

void Game::StartFinalScore(bool won) {
  state = GameState::FinalScore;
  playState.Finish(won);
}

void Game::HandleInput() {
  if (state != GameState::Playing) {
    return;
  }
  if (statusKinect.jump || IsKeyPressed(KEY_SPACE)) {
    if (fox.Jump()) {
      cameraEffects.TriggerJump();
    }
  }
}

void Game::SetKinectStatus() {

  if (statusKinect.lastStatusKinnect != statusKinect.tracked &&
      state != GameState::Result && state != GameState::FinalScore) {
    if (statusKinect.tracked) {
      PlaySound(trackedsound);
    } else {
      PlaySound(notTrackedSound);
    }
    statusKinect.lastStatusKinnect = statusKinect.tracked;
  }

  if (!statusKinect.tracked) {
    if (!playState.gameOver) {
      if (state == GameState::Playing) {
        statusKinect.waitingReconnect = true;
      }
      state = GameState::WaitingForPlayer;
      countdown.Reset();
    }
    return;
  }

  if (state == GameState::Result || state == GameState::FinalScore) {
    return;
  }

  if (state == GameState::WaitingForPlayer && statusKinect.tracked) {
    // Tracking sudah kembali
    if (statusKinect.waitingReconnect) {
      statusKinect.waitingReconnect = false;
      statusKinect.trackingLostTimer = 0.0f;
      statusKinect.holdTime = 0.0f; // opsional
    }
    if (statusKinect.readyPose &&
        statusKinect.holdTime >= statusKinect.READY_HOLD_REQUIRED) {
      StartCountdown();
    }
  }
}

void Game::Update(float dt) {

  SetKinectStatus();
  HandleInput();

  playState.systemScore.Update(dt);

  switch (state) {
  case GameState::WaitingForPlayer:
    UpdateWaitingForPlayer(dt);
    return;

  case GameState::Countdown:
    UpdateCountdown(dt);
    return;

  case GameState::Playing:
    UpdatePlaying(dt);
    return;

  case GameState::Result:
    UpdateResult(dt);
    return;

  case GameState::FinalScore:
    UpdateFinalScore(dt);
    return;
  }
}

void Game::UpdateWaitingForPlayer(float dt) {
  if (statusKinect.waitingReconnect) {
    statusKinect.trackingLostTimer += dt;

    if (statusKinect.trackingLostTimer >= statusKinect.TRACKING_LOST_LIMIT) {
      statusKinect.waitingReconnect = false;
      statusKinect.trackingLostTimer = 0.0f;

      fox.Die(Direction::Left);
      StartResult(false);
      return;
    }
  }

  parallax.Update(GetFrameTime());
  fox.UpdateIdle(dt);
  UpdateCamera();
}

void Game::UpdatePlaying(float dt) {
  parallax.Update(GetFrameTime());
  fox.Update(dt);
  stars.Update(dt);

  playState.survivalTimer -= dt;
  if (playState.survivalTimer <= 0.0f) {
    StartResult(true);
    return;
  }

  UpdateGameplay(dt);
  UpdateCamera();
}

void Game::UpdateGameplay(float dt) {
  PlatformEvent event =
      platforms.Update(dt, fox.GetPreviousPosition(), fox.GetPosition(),
                       fox.IsFalling(), fox.IsGrounded());

  HandleEvent(event);
}

void Game::UpdateResult(float dt) {
  parallax.Update(GetFrameTime());
  fox.Update(dt);
  stars.Update(dt);
  UpdateCamera();

  if (result.Update(dt)) {
    StartFinalScore(result.won);
  }
}

void Game::UpdateFinalScore(float dt) {
  fox.UpdateIdle(dt);
  if (playState.FinalTimerIsEnd(dt)) {
    Reset();
  }
}

void Game::HandleEvent(const PlatformEvent &event) {
  switch (event.type) {
  case PlatformEventType::Landed: {

    if (event.perfectLanding) {
      PlaySound(perfectsound);
      playState.UpdateScore(true);
    } else {
      playState.UpdateScore(false);
    }

    fox.Face(event.direction);
    fox.Land(event.landingY);
    stars.Spawn(fox.GetPosition(), event.perfectLanding ? 1.15f : 0.8f);
    cameraEffects.TriggerLanding(event.perfectLanding, event.direction);
    break;
  }

  case PlatformEventType::Hit: {
    playState.systemScore.ResetCombo();
    fox.Die(event.direction);
    StartResult(false);
    cameraEffects.TriggerHit();
    break;
  }
  case PlatformEventType::JumpedTooEarly: {
    playState.systemScore.ResetCombo();
    fox.Die(event.direction, fox.GetJumpStartY());
    // TraceLog(LOG_DEBUG, "EVENT %s", "JUMPED_TOO_EARLY");
    StartResult(false);
    cameraEffects.TriggerHit();
    break;
  }
  case PlatformEventType::PlatformPassed: {
    playState.systemScore.ResetCombo();
    fox.Die(event.direction, fox.GetJumpStartY());
    // TraceLog(LOG_DEBUG, "EVENT %s", "PLATFORM_PASSED");
    StartResult(false);
    cameraEffects.TriggerHit();
    break;
  }
  default:
    break;
  }
}

void Game::UpdateCamera() {
  camera.fovy = 45.0f;
  camera.up = {0, 1, 0};
  camera.position.x = 0.0f;
  camera.position.z = CAMERA_DISTANCE;

  cameraHeight = Lerp(cameraHeight, platforms.GetHeight(), CAMERA_FOLLOW_SPEED);

  Vector3 target = {0.0f, cameraHeight + CAMERA_TARGET_OFFSET, 0.0f};

  camera.target = Vector3Lerp(camera.target, target, CAMERA_FOLLOW_SPEED);

  camera.position.y =
      Lerp(camera.position.y, cameraHeight + CAMERA_HEIGHT_OFFSET,
           CAMERA_FOLLOW_SPEED);

  cameraEffects.Update(GetFrameTime(), !fox.IsGrounded());
  cameraEffects.Apply(camera);
}
void Game::UpdateCountdown(float dt) {
  parallax.Update(GetFrameTime());
  fox.Update(dt);
  stars.Update(dt);
  if (countdown.Update(dt) && countdown.current >= 0) {
    PlaySound(countdown.current > 0 ? countsound : goSound);
  }

  if (countdown.IsOver()) {
    state = GameState::Playing;
    playState.survivalTimer = playState.systemScore.SURVIVAL_TIME_LIMIT;
  }
}
void Game::DrawCountdown() {
  hud.DrawCountdown(countdown.current, countdown.Progress());
}
void Game::Draw() {
  if (state == GameState::FinalScore) {
    DrawFinalScoreScene();
    return;
  }

  DrawGameplayScene();

  switch (state) {
  case GameState::Playing:
    DrawSurvivalTimer();
    DrawKinectInstructions();
    break;

  case GameState::Countdown:
    DrawCountdown();
    DrawKinectInstructions();
    break;

  case GameState::Result:
    DrawResultOverlay();
    break;

  case GameState::WaitingForPlayer:
    DrawKinectInstructions();
    break;

  case GameState::FinalScore:
    break;
  }
}

void Game::DrawGameplayScene() {
  parallax.Draw(camera.target.y);
  BeginMode3D(camera);
  DrawModelEx(island, {0, -0.3f, 0}, {0, 1, 0}, 0.0f, Vector3One(), WHITE);
  platforms.Draw();
  fox.Draw();
  EndMode3D();

  stars.Draw(camera);
  DrawLandingFeedback();
}

void Game::DrawSurvivalTimer() { hud.DrawTopHUD(playState.survivalTimer); }

void Game::DrawResultOverlay() {
  const char *resultText = result.won ? "BERHASIL" : "GAGAL";
  const Color resultColor = result.won ? GREEN : RED;
  const int fontSize = 72;
  const int textWidth = MeasureText(resultText, fontSize);

  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.35f));
  DrawText(resultText, GetScreenWidth() / 2 - textWidth / 2,
           GetScreenHeight() / 2 - fontSize / 2, fontSize, resultColor);
}

void Game::DrawLandingFeedback() { hud.Draw(fox.GetPosition()); }

void Game::DrawFinalScoreScene() { hud.DrawFinalScoreScene(); }

void Game::DrawKinectInstructions() {
  kinectInstruction.Draw(state, playState.gameOver);
}

void Game::Shutdown() {
  parallax.Unload();
  hud.Unload();
  UnloadSound(countsound);
  UnloadSound(goSound);
  UnloadSound(notTrackedSound);
  UnloadSound(trackedsound);
  UnloadSound(perfectsound);
  UnloadModel(island);
  // UnloadFont(poppins);
  //  UnloadTexture(skyboxtex);
}
