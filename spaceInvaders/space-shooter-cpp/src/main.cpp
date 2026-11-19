#include "raylib.h"
#include <algorithm>
#include <array>
#include <random>

constexpr int SCREEN_W = 800;
constexpr int SCREEN_H = 600;
constexpr size_t MAX_BULLETS = 32;
constexpr size_t MAX_ENEMIES = 24;

struct Bullet {
    Vector2 pos{0.0f, 0.0f};
    bool active = false;
};

struct Enemy {
    Vector2 pos{0.0f, 0.0f};
    float speed = 150.0f;
    float radius = 18.0f;
    bool active = false;
};

int main() {
    InitWindow(SCREEN_W, SCREEN_H, "C++ + Raylib Space Shooter");
    SetTargetFPS(60);

    // Random number generator
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> distX(30.0f, SCREEN_W - 30.0f);
    std::uniform_real_distribution<float> distSpeed(140.0f, 260.0f);

    // Player state
    Vector2 playerPos{SCREEN_W / 2.0f, SCREEN_H - 60.0f};
    constexpr float playerSpeed = 380.0f;
    constexpr float playerRadius = 16.0f;

    // Entity pools
    std::array<Bullet, MAX_BULLETS> bullets{};
    std::array<Enemy, MAX_ENEMIES> enemies{};

    // Timers & Game State
    float shootCooldown = 0.0f;
    float spawnTimer = 0.0f;
    int score = 0;
    bool gameOver = false;

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();

        // --- UPDATE ---
        if (!gameOver) {
            // 1. Player Movement (WASD or Arrow Keys)
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  playerPos.x -= playerSpeed * dt;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) playerPos.x += playerSpeed * dt;
            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    playerPos.y -= playerSpeed * dt;
            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  playerPos.y += playerSpeed * dt;

            // Clamp player to screen bounds
            playerPos.x = std::clamp(playerPos.x, playerRadius, static_cast<float>(SCREEN_W) - playerRadius);
            playerPos.y = std::clamp(playerPos.y, playerRadius, static_cast<float>(SCREEN_H) - playerRadius);

            // 2. Shooting
            shootCooldown -= dt;
            if (IsKeyDown(KEY_SPACE) && shootCooldown <= 0.0f) {
                for (auto& b : bullets) {
                    if (!b.active) {
                        b.active = true;
                        b.pos = {playerPos.x, playerPos.y - 20.0f};
                        shootCooldown = 0.18f;
                        break;
                    }
                }
            }

            // 3. Update Bullets
            for (auto& b : bullets) {
                if (!b.active) continue;
                b.pos.y -= 650.0f * dt;
                if (b.pos.y < -10.0f) b.active = false;
            }

            // 4. Spawn Enemies
            spawnTimer -= dt;
            if (spawnTimer <= 0.0f) {
                for (auto& e : enemies) {
                    if (!e.active) {
                        e.active = true;
                        e.pos = {distX(rng), -20.0f};
                        e.speed = distSpeed(rng);
                        spawnTimer = 0.65f;
                        break;
                    }
                }
            }

            // 5. Update Enemies & Collisions
            for (auto& e : enemies) {
                if (!e.active) continue;
                e.pos.y += e.speed * dt;

                if (e.pos.y > SCREEN_H + 25.0f) {
                    e.active = false;
                    continue;
                }

                // Player Collision
                if (CheckCollisionCircles(playerPos, playerRadius, e.pos, e.radius)) {
                    gameOver = true;
                }

                // Bullet Collisions
                for (auto& b : bullets) {
                    if (!b.active) continue;
                    if (CheckCollisionCircles(b.pos, 5.0f, e.pos, e.radius)) {
                        b.active = false;
                        e.active = false;
                        score += 100;
                        break;
                    }
                }
            }
        } else if (IsKeyPressed(KEY_R)) {
            // Reset Game
            playerPos = {SCREEN_W / 2.0f, SCREEN_H - 60.0f};
            for (auto& b : bullets) b.active = false;
            for (auto& e : enemies) e.active = false;
            score = 0;
            gameOver = false;
        }

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw Player Ship (Triangle)
        DrawTriangle(
            {playerPos.x, playerPos.y - 20.0f},
            {playerPos.x - 16.0f, playerPos.y + 16.0f},
            {playerPos.x + 16.0f, playerPos.y + 16.0f},
            SKYBLUE
        );

        // Draw Bullets
        for (const auto& b : bullets) {
            if (b.active) {
                DrawRectangleV({b.pos.x - 2.0f, b.pos.y - 8.0f}, {4.0f, 14.0f}, YELLOW);
            }
        }

        // Draw Enemies
        for (const auto& e : enemies) {
            if (e.active) {
                DrawCircleV(e.pos, e.radius, RED);
                DrawCircleLines(static_cast<int>(e.pos.x), static_cast<int>(e.pos.y), e.radius + 3.0f, ORANGE);
            }
        }

        // Draw UI
        DrawText(TextFormat("SCORE: %d", score), 16, 16, 20, WHITE);

        if (gameOver) {
            DrawText("GAME OVER", SCREEN_W / 2 - 110, SCREEN_H / 2 - 30, 40, RED);
            DrawText("Press [R] to Restart", SCREEN_W / 2 - 105, SCREEN_H / 2 + 20, 20, LIGHTGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
