#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>
#include <vector>

enum class GameScreen {
  UNKNOWN,
  TITLE,
  INTRO,
  OPTIONS,
  GAMEPLAY,
  ENDING,
  GAMEOVER
};

struct PlayerStats {
    int hits = 0;              // Number of collectibles hit
    int keystrokes = 0;        // Number of movements (keystrokes or inputs)
    int collisions = 0;        // Number of collisions with obstacles
    int score = 0;             // Player's score
    float playtime = 0.0f;     // Playtime in seconds

    void Reset() {
        hits = 0;
        keystrokes = 0;
        collisions = 0;
        score = 0;
        playtime = 0.0f;
    }
};

enum class EntityType { OBSTACLE, COLLECTIBLE };

struct Entity {
  Vector3 position;
  Vector3 size;
  Color color;
  EntityType type;

  Entity(Vector3 pos, Vector3 s, Color c, EntityType t)
      : position(pos), size(s), color(c), type(t) {}
};

class Game {
public:
  Game()
      : currentScreen(GameScreen::TITLE), transAlpha(0.0f), transFadeOut(false),
        onTransition(false), transFromScreen(GameScreen::UNKNOWN),
        transToScreen(GameScreen::UNKNOWN), playerPosition{0.0f, 0.0f, -5.0f},
        initialPlayerSpeed(5.0f),
        playerSpeed(5.0f) {}

  void Init() {
    // SetConfigFlags (FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    SetConfigFlags(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED);
    InitWindow(screenWidth, screenHeight,
               "Endless Scroller with Obstacles and Collectibles");
    SetTargetFPS(60);
    rlImGuiSetup(true);
    InitEntities();
    planetFloorShader = LoadShader(0, "resources/shaders/gsl330/planet_plane.fs");
    skyShader = LoadShader(0, "resources/shaders/gsl330/lunar_sky.fs");

  }

  void Shutdown() {
    rlImGuiShutdown();
    CloseWindow();
  }

  void Run() {
    while (!WindowShouldClose()) {
      Update();
      Draw();
    }
  }

private:
  GameScreen currentScreen;
  GameScreen transFromScreen;
  GameScreen transToScreen;

  bool onTransition;
  bool transFadeOut;
  float transAlpha;

  int screenWidth = 800;
  int screenHeight = 420;

  Vector3 playerPosition;
  float initialPlayerSpeed;
  float playerSpeed;

  std::vector<Entity> entities;

  PlayerStats stats;

  Shader planetFloorShader;
  Shader skyShader;
  // lighning
  float lightningTimer = 0.0f;
  float lightningFlashDuration = 0.2f;
  float lightningCooldown = 3.0f; // Time between lightning strikes
  bool isLightningActive = false;

  void InitEntities() {
    // Add some obstacles
    for (int i = 0; i < 5; ++i) {
      entities.emplace_back(Vector3{-2.0f + i * 2.0f, 0.0f, -10.0f - i * 5.0f},
                            Vector3{1.0f, 1.0f, 1.0f}, RED,
                            EntityType::OBSTACLE);
    }

    // Add some collectibles
    for (int i = 0; i < 5; ++i) {
      entities.emplace_back(Vector3{2.0f - i * 2.0f, 0.0f, -12.0f - i * 5.0f},
                            Vector3{0.5f, 0.5f, 0.5f}, GREEN,
                            EntityType::COLLECTIBLE);
    }
  }

  void Update() {
    if (onTransition) {
      UpdateTransition();
    } else {
      switch (currentScreen) {
      case GameScreen::TITLE:
        UpdateTitleScreen();
        break;
      case GameScreen::GAMEPLAY:
        UpdateGameplayScreen();
        break;
      case GameScreen::GAMEOVER:
        UpdateGameOverScreen();
      default:
        break;
      }
    }
  }

  void Draw() {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    if (!onTransition) {
      switch (currentScreen) {
      case GameScreen::TITLE:
        DrawTitleScreen();
        break;
      case GameScreen::GAMEPLAY:
        DrawGameplayScreen();
        break;
      case GameScreen::GAMEOVER:
        DrawGameOverScreen();
        break;
      default:
        break;
      }
    } else {
      DrawTransition();
    }

    rlImGuiBegin();
    DrawImgui();
    rlImGuiEnd();

    EndDrawing();
  }

  void UpdateTitleScreen() {
    if (IsKeyPressed(KEY_ENTER)) {
      TransitionToScreen(GameScreen::GAMEPLAY);
    }

    if (IsKeyDown(KEY_F)) {
        ToggleFullscreen();
        // MaximizeWindow();
    }
  }

  void UpdateGameOverScreen() {
      if(IsKeyPressed(KEY_ENTER)) {
          InitGameplay();
          TransitionToScreen(GameScreen::TITLE);
      }
  }

  void DrawTitleScreen() {
    DrawText("Endless Scroller - Press [ENTER] to Start", 100, 200, 20, DARKBLUE);
    DrawText("                 - Press [f] to toggle Fullscreen", 100, 300, 10, DARKBLUE);
  }

  void DrawSky() {
    BeginShaderMode(skyShader);
    DrawSphere(Vector3{0,0,0}, 100.0f, WHITE);
    EndShaderMode();
    if (isLightningActive) {
        DrawLightningBolt(Vector3{0.0f + GetRandomValue(-10, 10), 5.0f, -10.0f},
                          Vector3{0.0f + GetRandomValue(-10, 10), 0.0f, -10.0f});
    }
  }

  void DrawLightningBolt(Vector3 startPos, Vector3 endPos) {
      DrawLine3D(startPos, endPos, WHITE);
  }

  void DrawGameOverScreen() {
    DrawText("Press [ENTER] to restart ...", 100, 50, 20, DARKBLUE);

    // Display detailed stats
    DrawText(TextFormat("Final Score: %d", stats.score), 100, 100, 20, DARKPURPLE);
    DrawText(TextFormat("Total Hits: %d", stats.hits), 100, 130, 20, DARKGREEN);
    DrawText(TextFormat("Total Keystrokes: %d", stats.keystrokes), 100, 160, 20, DARKBLUE);
    DrawText(TextFormat("Total Collisions: %d", stats.collisions), 100, 190, 20, RED);
    DrawText(TextFormat("Playtime: %.2f seconds", stats.playtime), 100, 220, 20, ORANGE);

  }

  void UpdateGameplayScreen() {
    stats.playtime += GetFrameTime();
    bool inputMade = false;

    if (IsKeyDown(KEY_RIGHT)) {
      inputMade = true;
      playerPosition.x += playerSpeed * GetFrameTime();
    }
    if (IsKeyDown(KEY_LEFT)) {
      inputMade = true;
      playerPosition.x -= playerSpeed * GetFrameTime();
    }
    if (IsKeyDown(KEY_UP)) {
      inputMade = true;
      playerPosition.z -= playerSpeed * GetFrameTime();
    }
    if (IsKeyDown(KEY_DOWN)) {
      inputMade = true;
      playerPosition.z += playerSpeed * GetFrameTime();
    }

    if (inputMade) {
        stats.keystrokes++;
    }
    // playerPosition.z += playerSpeed * GetFrameTime();
    // Clamp Player bounds
    if (playerPosition.z > 2.0f) {
      playerPosition.z = 2.0f;
    } else if (playerPosition.z < -1.0f) {
        playerPosition.z = -1.0f;
    }
    if (playerPosition.x > 4.0f) {
      playerPosition.x = 4.0f;
    } else if (playerPosition.x < -4.0f) {
        playerPosition.x = -4.0f;
    }

    // Update Lightning
    lightningTimer += GetFrameTime();

    if (!isLightningActive && lightningTimer > lightningCooldown) {
        isLightningActive = true;
        lightningTimer = 0.0f; // Reset timer for flash duration
    }

    if (isLightningActive && lightningTimer > lightningFlashDuration) {
        isLightningActive = false;
        lightningCooldown = 2.0f + GetRandomValue(1, 5); // Randomize next strike
        lightningTimer = 0.0f;
    }

    // Endless scrolling logic

    // Update entity positions (endless scrolling effect)
    for (auto& entity : entities) {
      entity.position.z += playerSpeed * GetFrameTime();
      if (entity.position.z > 5.0f)
        entity.position.z = -20.0f;

      // Check collisions
      if (CheckCollision(playerPosition, entity)) {
        if (entity.type == EntityType::COLLECTIBLE) {
          // Collect the item
          stats.hits++;
          stats.score += 100;
          entity.position.z = -20.0f; // Respawn the collectible
        } else if (entity.type == EntityType::OBSTACLE) {
            stats.collisions++;
          // Handle collision with obstacle (e.g., reset position)
          TransitionToScreen(GameScreen::GAMEOVER);
        }
      }
    }
    playerSpeed += GetFrameTime() * 0.01;
  }

  void InitGameplay() {
    // set to initial player position
    // playerPosition.z = -5.0f;
    playerPosition = Vector3{0.0f, 0.0f, 0.5f};
    playerSpeed = initialPlayerSpeed;

    stats.Reset();

    // Update entity positions (endless scrolling effect)
    for (auto& entity : entities) {
      entity.position.z -= 20.0f; // Respawn the collectible
    }
  }

  void RewardCombo() {
      if (stats.hits % 3 == 0) {
          stats.score += 500;
      }
  }

  void DrawFloor() {
      // DrawGrid(10, 1.0f);
      BeginShaderMode(planetFloorShader);
      for (int i = 0; i < 10; ++i) { // Number of planes visible at a time
          Vector3 planePosition = Vector3{0.0f, 0.0f, playerPosition.z - (i * 10.0f)};
          DrawPlane(planePosition, Vector2{10.0f, 10.0f}, LIGHTGRAY);
      }
      EndShaderMode();
  }


  bool CheckCollision(const Vector3& player, const Entity& entity) {
    return CheckCollisionBoxes(
        BoundingBox{Vector3{player.x - 0.5f, player.y - 0.5f, player.z - 0.5f},
                    Vector3{player.x + 0.5f, player.y + 0.5f, player.z + 0.5f}},
        BoundingBox{Vector3{entity.position.x - entity.size.x / 2,
                            entity.position.y - entity.size.y / 2,
                            entity.position.z - entity.size.z / 2},
                    Vector3{entity.position.x + entity.size.x / 2,
                            entity.position.y + entity.size.y / 2,
                            entity.position.z + entity.size.z / 2}});
  }

  void DrawGameplayScreen() {

    BeginMode3D(Camera{
        Vector3{0.0f, 2.0f, 6.0f}, // Camera position
        Vector3{0.0f, 1.0f, 0.0f}, // Target position
        Vector3{0.0f, 1.0f, 0.0f}, // Up vector
        45.0f,                     // FOV
        CAMERA_PERSPECTIVE,        // Camera mode
    });

    DrawSky();
    DrawFloor();

    // Draw the player player
    DrawCube(playerPosition, 1.0f, 1.0f, 1.0f, BLUE);

    // Draw entities
    for (const auto& entity : entities) {
      DrawCube(entity.position, entity.size.x, entity.size.y, entity.size.z,
               entity.color);
    }

    EndMode3D();

    // HUD
    DrawText(TextFormat("Score: %d", stats.score), screenWidth - 200, 10, 20, BLACK);
    DrawText(TextFormat("Hits: %d", stats.hits), screenWidth - 200, 40, 20, BLACK);
    DrawText(TextFormat("Keystrokes: %d", stats.keystrokes), screenWidth - 200, 70, 20, BLACK);
    DrawText(TextFormat("Collisions: %d / 3", stats.collisions), screenWidth - 200, 100, 20, RED);
    DrawText(TextFormat("Playtime: %.2f s", stats.playtime), screenWidth - 200, 130, 20, DARKGREEN);

  }

  void TransitionToScreen(GameScreen screen) {
    onTransition = true;
    transFadeOut = false;
    transFromScreen = currentScreen;
    transToScreen = screen;
    transAlpha = 0.0f;
  }

  void UpdateTransition() {
    if (!transFadeOut) {
      transAlpha += 0.02f;
      if (transAlpha > 1.0f) {
        currentScreen = transToScreen;
        transFadeOut = true;
      }
    } else {
      transAlpha -= 0.02f;
      if (transAlpha < 0.0f) {
        onTransition = false;
        transAlpha = 0.0f;
      }
    }
  }

  void DrawTransition() const {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  Fade(BLACK, transAlpha));
  }
  void DrawImgui() const {
      ImGui::Begin("Realtime Panel");

      // Display the player's position
      ImGui::Text("Player Position:");
      ImGui::Text("X: %.2f", playerPosition.x);
      ImGui::Text("Y: %.2f", playerPosition.y);
      ImGui::Text("Z: %.2f", playerPosition.z);

      ImGui::Text("Player Stats:");
      ImGui::Text("Score: %d", stats.score);
      ImGui::Text("Hits: %d", stats.hits);
      ImGui::Text("Keystrokes: %d", stats.keystrokes);
      ImGui::Text("Collisions: %d / 3", stats.collisions);

      ImGui::SliderFloat("Player Speed", const_cast<float*>(&playerSpeed), 0.0f, 10.0f, "%.2f");

      ImGui::End();
  }


};

int main() {
  Game game;
  game.Init();
  game.Run();
  game.Shutdown();
  return 0;
}
