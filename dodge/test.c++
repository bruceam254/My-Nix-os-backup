#include "raylib.h"
#include <vector>

struct Enemy {
    Rectangle rect;
    Vector2 speed;
};

// Added MENU state before PLAYING
enum GameState {
    MENU,
    PLAYING,
    HIT_PROMPT,
    GAME_OVER
};

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "C++ Raylib Coin Collector");
    InitAudioDevice();
    SetTargetFPS(60);

    // Assets
    Texture2D playerTex = LoadTexture("/home/skyz/Desktop/Cplus/assets/player.png");
    Sound coinSound = LoadSound("/home/skyz/Desktop/Cplus/assets/coin.mp3");

    // Player setup
    Rectangle player = { 400.0f, 225.0f, 40.0f, 40.0f };
    float playerSpeed = 300.0f;

    // Coin setup
    Vector2 coinPos = { 200.0f, 150.0f };
    float coinRadius = 15.0f;

    // Master list of 4 corner enemies (we slice the first 2, 3, or 4 based on player choice)
    const std::vector<Enemy> allCornerEnemies = {
        { { 60.0f,  60.0f,  30.0f, 30.0f }, {  220.0f,  180.0f } }, // 1. Top-left
        { { 700.0f, 350.0f, 30.0f, 30.0f }, { -200.0f, -210.0f } }, // 2. Bottom-right
        { { 700.0f, 60.0f,  30.0f, 30.0f }, { -240.0f,  190.0f } }, // 3. Top-right
        { { 60.0f,  350.0f, 30.0f, 30.0f }, {  210.0f, -220.0f } }  // 4. Bottom-left
    };

    int selectedEnemyCount = 3;
    std::vector<Enemy> activeEnemies;

    // Score, Lives & State tracking
    int score = 0;
    int previousScore = 0;
    int highScore = 0;
    int lives = 3;
    float timeLeft = 30.0f;
    GameState state = MENU; // Start at the selection screen

    // Helper to build the chosen number of enemies (2, 3, or 4)
    auto spawnSelectedEnemies = [&]() {
        activeEnemies.clear();
        for (int i = 0; i < selectedEnemyCount; i++) {
            activeEnemies.push_back(allCornerEnemies[i]);
        }
    };

    // Helper to start or reset a full run
    auto startNewRun = [&](bool savePreviousScore) {
        if (savePreviousScore) {
            previousScore = score;
            if (score > highScore) {
                highScore = score;
            }
        }
        player.x = 400.0f;
        player.y = 225.0f;
        coinPos = { 200.0f, 150.0f };
        spawnSelectedEnemies();
        score = 0;
        lives = 3;
        timeLeft = 30.0f;
        state = PLAYING;
    };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (state == MENU) {
            if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) {
                selectedEnemyCount = 2;
                startNewRun(false);
            } else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) {
                selectedEnemyCount = 3;
                startNewRun(false);
            } else if (IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_KP_4)) {
                selectedEnemyCount = 4;
                startNewRun(false);
            }
        }
        else if (state == PLAYING) {
            // 1. Update Timer
            timeLeft -= dt;
            if (timeLeft <= 0.0f) {
                timeLeft = 0.0f;
                if (score > highScore) highScore = score;
                state = GAME_OVER;
            }

            // 2. Player Movement
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.x += playerSpeed * dt;
            if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) player.x -= playerSpeed * dt;
            if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) player.y += playerSpeed * dt;
            if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) player.y -= playerSpeed * dt;

            // Keep player on screen
            if (player.x < 0) player.x = 0;
            if (player.x > screenWidth - player.width) player.x = screenWidth - player.width;
            if (player.y < 0) player.y = 0;
            if (player.y > screenHeight - player.height) player.y = screenHeight - player.height;

            // 3. Move & Bounce Enemies
            for (auto& enemy : activeEnemies) {
                enemy.rect.x += enemy.speed.x * dt;
                enemy.rect.y += enemy.speed.y * dt;

                if (enemy.rect.x <= 0 || enemy.rect.x + enemy.rect.width >= screenWidth) {
                    enemy.speed.x *= -1;
                }
                if (enemy.rect.y <= 0 || enemy.rect.y + enemy.rect.height >= screenHeight) {
                    enemy.speed.y *= -1;
                }

                // Check Enemy Collision
                if (CheckCollisionRecs(player, enemy.rect)) {
                    if (lives > 1) {
                        state = HIT_PROMPT;
                    } else {
                        lives = 0;
                        if (score > highScore) highScore = score;
                        state = GAME_OVER;
                    }
                    break;
                }
            }

            // 4. Check collision between player and coin
            if (CheckCollisionCircleRec(coinPos, coinRadius, player)) {
                score++;
                if (score > highScore) highScore = score;
                PlaySound(coinSound);
                coinPos.x = (float)GetRandomValue(20, screenWidth - 20);
                coinPos.y = (float)GetRandomValue(20, screenHeight - 20);
            }
        }
        else if (state == HIT_PROMPT) {
            // Option 1: Use a life (keep score & timer, reset positions safely)
            if (IsKeyPressed(KEY_L) || IsKeyPressed(KEY_SPACE)) {
                lives--;
                player.x = 400.0f;
                player.y = 225.0f;
                spawnSelectedEnemies(); // Resets the chosen 2, 3, or 4 enemies to corners
                state = PLAYING;
            }
            // Option 2: Full Reset with same enemy count
            else if (IsKeyPressed(KEY_R)) {
                startNewRun(true);
            }
        }
        else if (state == GAME_OVER) {
            if (IsKeyPressed(KEY_R)) {
                startNewRun(true);
            } else if (IsKeyPressed(KEY_M)) {
                previousScore = score;
                if (score > highScore) highScore = score;
                state = MENU; // Return to enemy count selection
            }
        }

        // 5. Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (state == MENU) {
            DrawText("COIN COLLECTOR", screenWidth / 2 - 155, 80, 36, DARKBLUE);
            DrawText("Select Number of Enemies to Start:", screenWidth / 2 - 195, 160, 22, DARKGRAY);

            DrawText("Press [2] - Easy   (2 Enemies)", screenWidth / 2 - 165, 215, 22, DARKGREEN);
            DrawText("Press [3] - Normal (3 Enemies)", screenWidth / 2 - 165, 250, 22, ORANGE);
            DrawText("Press [4] - Hard   (4 Enemies)", screenWidth / 2 - 165, 285, 22, RED);

            if (highScore > 0 || previousScore > 0) {
                DrawText(TextFormat("High Score: %i   |   Previous Score: %i", highScore, previousScore), screenWidth / 2 - 185, 360, 20, GRAY);
            }
        }
        else if (state == PLAYING || state == HIT_PROMPT) {
            if (playerTex.id > 0) {
                DrawTexturePro(
                    playerTex,
                    { 0.0f, 0.0f, (float)playerTex.width, (float)playerTex.height },
                    player,
                    { 0.0f, 0.0f },
                    0.0f,
                    WHITE
                );
            } else {
                DrawRectangleRec(player, BLUE);
            }

            DrawCircleV(coinPos, coinRadius, GOLD);

            for (const auto& enemy : activeEnemies) {
                DrawRectangleRec(enemy.rect, RED);
            }

            // Top HUD
            DrawText(TextFormat("Score: %i", score), 20, 15, 22, DARKGRAY);
            DrawText(TextFormat("Lives: %i", lives), 20, 42, 22, MAROON);
            DrawText(TextFormat("High: %i  |  Prev: %i", highScore, previousScore), 255, 15, 20, GRAY);
            DrawText(TextFormat("Enemies: %i", selectedEnemyCount), 335, 42, 18, DARKGRAY);
            DrawText(TextFormat("Time: %.1f", timeLeft), screenWidth - 140, 15, 22, DARKGRAY);
        }

        if (state == HIT_PROMPT) {
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.55f));
            DrawText("YOU WERE HIT!", screenWidth / 2 - 125, screenHeight / 2 - 60, 32, ORANGE);
            DrawText(TextFormat("Lives Remaining: %i", lives - 1), screenWidth / 2 - 105, screenHeight / 2 - 15, 22, RAYWHITE);
            DrawText("Press [L] to Use a Life (Keep Score)", screenWidth / 2 - 185, screenHeight / 2 + 25, 20, GREEN);
            DrawText("Press [R] to Reset Game", screenWidth / 2 - 125, screenHeight / 2 + 55, 20, LIGHTGRAY);
        }
        else if (state == GAME_OVER) {
            DrawText("GAME OVER!", screenWidth / 2 - 110, screenHeight / 2 - 90, 36, RED);
            DrawText(TextFormat("Final Score: %i", score), screenWidth / 2 - 90, screenHeight / 2 - 40, 24, DARKGRAY);
            DrawText(TextFormat("Previous Score: %i   |   High Score: %i", previousScore, highScore), screenWidth / 2 - 190, screenHeight / 2, 20, GRAY);
            DrawText("Press [R] to Play Again (Same Enemies)", screenWidth / 2 - 205, screenHeight / 2 + 45, 20, DARKGRAY);
            DrawText("Press [M] to Change Enemy Count", screenWidth / 2 - 175, screenHeight / 2 + 75, 20, DARKBLUE);
        }

        EndDrawing();
    }

    // Cleanup
    UnloadTexture(playerTex);
    UnloadSound(coinSound);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
