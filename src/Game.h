#pragma once

#include "ArkanoidPlugin.h"

#include <cstdint>

#include <SDL3/SDL.h>
#include <vector>
#include <random>

#include "GameConfig.h"
#include "PluginManager.h"

#include <filesystem>

class PluginManager;

class Game
{
    friend class PluginManager;

public:
    enum class State
    {
        Ready,
        Playing,
        Won,
        GameOver
    };

public:
    Game(SDL_Renderer* renderer,
         const GameConfig& config);

    void ProcessEvent(const SDL_Event& event);
    void Update(float deltaTime);
    void Render() const;

    [[nodiscard]]
    bool IsRunning() const
    {
        return m_Running;
    }

    [[nodiscard]]
    std::uint32_t GetBrickCount() const;

    [[nodiscard]]
    std::uint32_t GetBrickColumns() const;

    [[nodiscard]]
    std::uint32_t GetBrickRows() const;

    [[nodiscard]]
    bool GetBrickInfo(
        std::uint32_t index,
        ArkanoidBrickInfo& outBrick) const;

    [[nodiscard]]
    bool GetBrickInfoAt(
        std::uint32_t column,
        std::uint32_t row,
        ArkanoidBrickInfo& outBrick) const;

    void LoadPlugins(
        const std::filesystem::path& basePath);

private:
    struct Collision
    {
        bool hit = false;

        SDL_FPoint normal{};
        float penetration = 0.0f;
    };

    struct Paddle
    {
        SDL_FRect bounds{};

        float speed = 0.0f;
        float velocityX = 0.0f;
    };

    struct Ball
    {
        SDL_FPoint position{};
        SDL_FPoint velocity{};

        float radius = 0.0f;
        float speed = 0.0f;

        bool active = false;
    };

    struct Brick
    {
        std::uint32_t id = 0;

        std::uint32_t row = 0;
        std::uint32_t column = 0;
        std::uint32_t typeId = 0;

        SDL_FRect bounds{};
        SDL_Color color{};

        int score = 0;
        bool active = true;
    };

private:
    void InitializeLevel();

    void UpdatePaddle(float deltaTime);
    void UpdateBall(float deltaTime);

    void RenderPaddle() const;
    void RenderBall() const;
    void RenderBricks() const;
    void RenderHud() const;

    void LaunchBall();

    [[nodiscard]]
    static Collision CircleVsRect(
        const SDL_FPoint& center,
        float radius,
        const SDL_FRect& rect);

    static void ResolveBallCollision(
        Ball& ball,
        const Collision& collision);

    void HandlePaddleCollision();
    void HandleBrickCollision();

    void RestartGame();

    [[nodiscard]]
    static ArkanoidBrickInfo MakeBrickInfo(
        const Brick& brick);

    void SetBrickColor(
        std::uint32_t brickId,
        std::uint8_t red,
        std::uint8_t green,
        std::uint8_t blue);

    void NormalizeBallVelocity();

private:
    GameConfig m_Config;

    SDL_Renderer* m_Renderer = nullptr;

    bool m_Running = true;

    Paddle m_Paddle{};
    Ball m_Ball{};

    std::vector<Brick> m_Bricks;

    State m_State = State::Ready;

    int m_Score = 0;
    int m_BallsRemaining = 3;


    std::mt19937 m_RandomEngine{
        std::random_device{}()
    };

    std::uint32_t m_BrickColumns = 0;
    std::uint32_t m_BrickRows = 0;

    PluginManager m_PluginManager;
};
