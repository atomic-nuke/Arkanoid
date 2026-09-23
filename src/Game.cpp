#include "Game.h"

#include <algorithm>
#include <cmath>

Game::Game(
    SDL_Renderer* renderer,
    const GameConfig& config)
    : m_Config(config)
      , m_Renderer(renderer)
      , m_PluginManager(*this)
{
    m_Paddle.bounds =
    {
        (m_Config.window.width - m_Config.paddle.width) * 0.5f,
        m_Config.window.height - m_Config.paddle.bottomOffset,
        m_Config.paddle.width,
        m_Config.paddle.height
    };

    m_Paddle.speed =
        m_Config.paddle.speed;

    m_Ball.radius =
        m_Config.ball.radius;

    m_Ball.speed =
        m_Config.ball.speed;

    m_Ball.position =
    {
        m_Config.window.width * 0.5f,
        m_Paddle.bounds.y -
        m_Ball.radius -
        2.0f
    };

    m_BallsRemaining =
        m_Config.ball.count;

    InitializeLevel();
}

void Game::InitializeLevel()
{
    m_Bricks.clear();

    m_BrickColumns =
        static_cast<std::uint32_t>(
            m_Config.bricks.columns);

    m_BrickRows = 0;

    const float totalWidth =
        m_Config.bricks.columns *
        m_Config.bricks.width +
        (m_Config.bricks.columns - 1) *
        m_Config.bricks.spacing;

    const float startX =
    (m_Config.window.width -
        totalWidth) * 0.5f;

    std::uint32_t id = 0;
    std::uint32_t row = 0;

    for (std::uint32_t typeId = 0;
         typeId < m_Config.bricks.types.size();
         ++typeId)
    {
        const GameConfig::BrickType& type =
            m_Config.bricks.types[typeId];

        for (int typeRow = 0;
             typeRow < type.rows;
             ++typeRow)
        {
            for (std::uint32_t column = 0;
                 column < m_BrickColumns;
                 ++column)
            {
                Brick brick;

                brick.id = id++;
                brick.row = row;
                brick.column = column;
                brick.typeId = typeId;

                brick.bounds =
                {
                    startX +
                    column *
                    (m_Config.bricks.width +
                        m_Config.bricks.spacing),

                    m_Config.bricks.topOffset +
                    row *
                    (m_Config.bricks.height +
                        m_Config.bricks.spacing),

                    m_Config.bricks.width,
                    m_Config.bricks.height
                };

                brick.color =
                {
                    type.red,
                    type.green,
                    type.blue,
                    255
                };

                brick.score =
                    type.score;

                m_Bricks.push_back(brick);
            }

            ++row;
        }
    }

    m_BrickRows = row;
}

void Game::ProcessEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_QUIT)
    {
        m_Running = false;
        return;
    }

    if (event.type != SDL_EVENT_KEY_DOWN)
        return;

    if (event.key.key == SDLK_ESCAPE)
    {
        m_Running = false;
    }
    else if (event.key.key == SDLK_SPACE)
    {
        LaunchBall();
    }
    else if (event.key.key == SDLK_R)
    {
        if (m_State == State::Won ||
            m_State == State::GameOver)
        {
            RestartGame();
        }
    }
}

void Game::Update(const float deltaTime)
{
    if (m_State == State::Won ||
        m_State == State::GameOver)
    {
        return;
    }

    // Game simulation

    UpdatePaddle(deltaTime);

    if (m_State == State::Playing)
    {
        UpdateBall(deltaTime);
    }

    // Plugin fixed update
    //
    // Existing timed plugin effects are updated before
    // events generated during this physics step are delivered.

    m_PluginManager.FixedUpdate(
        deltaTime);

    // Deliver events generated during this fixed step.

    m_PluginManager.DispatchQueuedEvents();

    // Apply deferred changes requested by plugins.

    m_PluginManager.ApplyQueuedCommands();
}

void Game::UpdatePaddle(const float deltaTime)
{
    ArkanoidPaddleUpdateContext pluginContext{};

    pluginContext.deltaTime =
        deltaTime;

    pluginContext.speedMultiplier =
        1.0f;

    pluginContext.widthMultiplier =
        1.0f;

    m_PluginManager.ModifyPaddleUpdate(
        pluginContext);

    const bool* keyboard =
        SDL_GetKeyboardState(nullptr);

    float direction = 0.0f;

    if (keyboard[SDL_SCANCODE_LEFT])
        direction -= 1.0f;

    if (keyboard[SDL_SCANCODE_RIGHT])
        direction += 1.0f;

    const float effectiveSpeed =
        m_Paddle.speed *
        pluginContext.speedMultiplier;

    m_Paddle.velocityX =
        direction * effectiveSpeed;

    const float previousCenter =
        m_Paddle.bounds.x +
        m_Paddle.bounds.w * 0.5f;

    m_Paddle.bounds.w =
        m_Config.paddle.width *
        pluginContext.widthMultiplier;

    m_Paddle.bounds.x =
        previousCenter -
        m_Paddle.bounds.w * 0.5f;

    m_Paddle.bounds.x +=
        m_Paddle.velocityX * deltaTime;

    m_Paddle.bounds.x = std::clamp(
        m_Paddle.bounds.x,
        0.0f,
        static_cast<float>(m_Config.window.width) -
        m_Paddle.bounds.w);

    if (!m_Ball.active)
    {
        m_Ball.position.x =
            m_Paddle.bounds.x +
            m_Paddle.bounds.w * 0.5f;

        m_Ball.position.y =
            m_Paddle.bounds.y -
            m_Ball.radius -
            2.0f;
    }
}

void Game::UpdateBall(const float deltaTime)
{
    if (!m_Ball.active)
        return;

    ArkanoidBallUpdateContext pluginContext{};

    pluginContext.deltaTime =
        deltaTime;

    pluginContext.speedMultiplier =
        1.0f;

    m_PluginManager.ModifyBallUpdate(
        pluginContext);

    // Movement

    m_Ball.position.x +=
        m_Ball.velocity.x *
        pluginContext.speedMultiplier *
        deltaTime;

    m_Ball.position.y +=
        m_Ball.velocity.y *
        pluginContext.speedMultiplier *
        deltaTime;

    // Walls

    if (m_Ball.position.x - m_Ball.radius < 0.0f)
    {
        m_Ball.position.x =
            m_Ball.radius;

        m_Ball.velocity.x =
            std::abs(m_Ball.velocity.x);
    }

    if (m_Ball.position.x + m_Ball.radius >
        m_Config.window.width)
    {
        m_Ball.position.x =
            m_Config.window.width -
            m_Ball.radius;

        m_Ball.velocity.x =
            -std::abs(m_Ball.velocity.x);
    }

    if (m_Ball.position.y - m_Ball.radius < 0.0f)
    {
        m_Ball.position.y =
            m_Ball.radius;

        m_Ball.velocity.y =
            std::abs(m_Ball.velocity.y);
    }

    HandlePaddleCollision();
    HandleBrickCollision();

    // Ball lost

    if (m_Ball.position.y - m_Ball.radius >
        m_Config.window.height)
    {
        m_Ball.active = false;

        --m_BallsRemaining;

        if (m_BallsRemaining <= 0)
        {
            m_State = State::GameOver;

            m_PluginManager.QueueGameEnded(
                ARKANOID_GAME_OVER);
        }
        else
        {
            m_State = State::Ready;
        }
    }
}

void Game::LaunchBall()
{
    if (m_State != State::Ready)
        return;

    if (m_BallsRemaining <= 0)
        return;

    std::uniform_int_distribution<int> direction(0, 1);

    const float x =
        direction(m_RandomEngine) == 0
            ? -0.6f
            : 0.6f;

    constexpr float y = -0.8f;

    m_Ball.velocity =
    {
        x * m_Ball.speed,
        y * m_Ball.speed
    };

    m_Ball.active = true;
    m_State = State::Playing;
}

void Game::Render() const
{
    SDL_SetRenderDrawColor(
        m_Renderer,
        20,
        20,
        25,
        255
    );

    SDL_RenderClear(m_Renderer);

    RenderBricks();
    RenderPaddle();
    RenderBall();
    RenderHud();

    SDL_RenderPresent(m_Renderer);
}

void Game::RenderPaddle() const
{
    SDL_SetRenderDrawColor(
        m_Renderer,
        230,
        230,
        230,
        255
    );

    SDL_RenderFillRect(
        m_Renderer,
        &m_Paddle.bounds
    );
}

void Game::RenderBall() const
{
    SDL_SetRenderDrawColor(
        m_Renderer,
        255,
        220,
        100,
        255
    );

    const SDL_FRect ballRect =
    {
        m_Ball.position.x - m_Ball.radius,
        m_Ball.position.y - m_Ball.radius,
        m_Ball.radius * 2.0f,
        m_Ball.radius * 2.0f
    };

    SDL_RenderFillRect(
        m_Renderer,
        &ballRect
    );
}

void Game::RenderBricks() const
{
    for (const Brick& brick : m_Bricks)
    {
        if (!brick.active)
            continue;

        SDL_SetRenderDrawColor(
            m_Renderer,
            brick.color.r,
            brick.color.g,
            brick.color.b,
            brick.color.a
        );

        SDL_RenderFillRect(
            m_Renderer,
            &brick.bounds
        );
    }
}

void Game::RenderHud() const
{
    SDL_SetRenderDrawColor(
        m_Renderer,
        255,
        255,
        255,
        255);

    SDL_RenderDebugTextFormat(
        m_Renderer,
        20.0f,
        20.0f,
        "SCORE: %d",
        m_Score);

    SDL_RenderDebugTextFormat(
        m_Renderer,
        20.0f,
        36.0f,
        "BALLS: %d",
        m_BallsRemaining);

    if (m_State == State::Ready)
    {
        SDL_RenderDebugText(
            m_Renderer,
            m_Config.window.width * 0.5f - 44.0f,
            m_Config.window.height * 0.5f,
            "PRESS SPACE");

        SDL_RenderDebugText(
            m_Renderer,
            m_Config.window.width * 0.5f - 52.0f,
            m_Config.window.height * 0.5f + 16.0f,
            "(ESC TO QUIT)");
    }
    else if (m_State == State::Won)
    {
        SDL_RenderDebugText(
            m_Renderer,
            m_Config.window.width * 0.5f - 32.0f,
            m_Config.window.height * 0.5f,
            "YOU WON!");

        SDL_RenderDebugText(
            m_Renderer,
            m_Config.window.width * 0.5f - 116.0f,
            m_Config.window.height * 0.5f + 16.0f,
            "(R TO RESTART, ESC TO QUIT)");
    }
    else if (m_State == State::GameOver)
    {
        SDL_RenderDebugText(
            m_Renderer,
            m_Config.window.width * 0.5f - 36.0f,
            m_Config.window.height * 0.5f,
            "GAME OVER");

        SDL_RenderDebugText(
            m_Renderer,
            m_Config.window.width * 0.5f - 116.0f,
            m_Config.window.height * 0.5f + 16.0f,
            "(R TO RESTART, ESC TO QUIT)");
    }
}

Game::Collision Game::CircleVsRect(
    const SDL_FPoint& center,
    const float radius,
    const SDL_FRect& rect)
{
    const float closestX = std::clamp(
        center.x,
        rect.x,
        rect.x + rect.w);

    const float closestY = std::clamp(
        center.y,
        rect.y,
        rect.y + rect.h);

    const float dx = center.x - closestX;
    const float dy = center.y - closestY;

    const float distanceSquared =
        dx * dx + dy * dy;

    const float radiusSquared =
        radius * radius;

    if (distanceSquared > radiusSquared)
        return {};

    Collision collision;
    collision.hit = true;

    // Normal case - circle center is outside the rectangle

    if (distanceSquared > 0.0f)
    {
        const float distance =
            std::sqrt(distanceSquared);

        collision.normal =
        {
            dx / distance,
            dy / distance
        };

        collision.penetration =
            radius - distance;

        return collision;
    }

    // Special case - circle center is inside the rectangle.

    const float distanceLeft =
        center.x - rect.x;

    const float distanceRight =
        rect.x + rect.w - center.x;

    const float distanceTop =
        center.y - rect.y;

    const float distanceBottom =
        rect.y + rect.h - center.y;

    float nearestDistance = distanceLeft;

    collision.normal = {-1.0f, 0.0f};

    if (distanceRight < nearestDistance)
    {
        nearestDistance = distanceRight;
        collision.normal = {1.0f, 0.0f};
    }

    if (distanceTop < nearestDistance)
    {
        nearestDistance = distanceTop;
        collision.normal = {0.0f, -1.0f};
    }

    if (distanceBottom < nearestDistance)
    {
        nearestDistance = distanceBottom;
        collision.normal = {0.0f, 1.0f};
    }

    collision.penetration =
        radius + nearestDistance;

    return collision;
}

void Game::ResolveBallCollision(
    Ball& ball,
    const Collision& collision)
{
    ball.position.x +=
        collision.normal.x * collision.penetration;

    ball.position.y +=
        collision.normal.y * collision.penetration;

    const float velocityDotNormal =
        ball.velocity.x * collision.normal.x +
        ball.velocity.y * collision.normal.y;

    // Reflect only when the ball is traveling into the surface.

    if (velocityDotNormal >= 0.0f)
        return;

    ball.velocity.x -=
        2.0f * velocityDotNormal * collision.normal.x;

    ball.velocity.y -=
        2.0f * velocityDotNormal * collision.normal.y;
}

void Game::HandlePaddleCollision()
{
    if (m_Ball.velocity.y <= 0.0f)
        return;

    if (m_Ball.position.y >= m_Paddle.bounds.y)
        return;

    const Collision collision =
        CircleVsRect(
            m_Ball.position,
            m_Ball.radius,
            m_Paddle.bounds);

    if (!collision.hit)
        return;

    ResolveBallCollision(
        m_Ball,
        collision);

    ArkanoidPaddleReflectionContext pluginContext{};

    pluginContext.velocityX =
        m_Ball.velocity.x;

    pluginContext.velocityY =
        m_Ball.velocity.y;

    pluginContext.paddleVelocityX =
        m_Paddle.velocityX;

    m_PluginManager.ModifyPaddleReflection(
        pluginContext);

    m_Ball.velocity.x =
        pluginContext.velocityX;

    m_Ball.velocity.y =
        pluginContext.velocityY;

    NormalizeBallVelocity();
}

void Game::HandleBrickCollision()
{
    for (Brick& brick : m_Bricks)
    {
        if (!brick.active)
            continue;

        const Collision collision =
            CircleVsRect(
                m_Ball.position,
                m_Ball.radius,
                brick.bounds);

        if (!collision.hit)
            continue;

        ResolveBallCollision(
            m_Ball,
            collision);

        ArkanoidBrickHitContext pluginContext{};

        pluginContext.brick =
            MakeBrickInfo(brick);

        pluginContext.baseScore =
            brick.score;

        pluginContext.score =
            brick.score;

        pluginContext.velocityX =
            m_Ball.velocity.x;

        pluginContext.velocityY =
            m_Ball.velocity.y;

        m_PluginManager.ModifyBrickHit(
            pluginContext);

        m_Ball.velocity.x =
            pluginContext.velocityX;

        m_Ball.velocity.y =
            pluginContext.velocityY;

        NormalizeBallVelocity();

        brick.active = false;

        m_Score += pluginContext.score;

        ArkanoidBrickDestroyedEvent destroyedEvent{};

        destroyedEvent.brick =
            MakeBrickInfo(brick);

        destroyedEvent.awardedScore =
            pluginContext.score;

        m_PluginManager.QueueBrickDestroyed(
            destroyedEvent);

        const bool anyBrickActive =
            std::any_of(
                m_Bricks.begin(),
                m_Bricks.end(),
                [](const Brick& candidate)
                {
                    return candidate.active;
                });

        if (!anyBrickActive)
        {
            m_State = State::Won;
            m_Ball.active = false;

            m_PluginManager.QueueGameEnded(
                ARKANOID_GAME_WON);
        }

        break;
    }
}

void Game::RestartGame()
{
    m_Score = 0;

    m_BallsRemaining =
        m_Config.ball.count;

    m_Paddle.bounds.x =
    (m_Config.window.width -
        m_Paddle.bounds.w) * 0.5f;

    m_Paddle.velocityX = 0.0f;

    m_Ball.active = false;
    m_Ball.velocity = {0.0f, 0.0f};

    m_Ball.position =
    {
        m_Paddle.bounds.x +
        m_Paddle.bounds.w * 0.5f,

        m_Paddle.bounds.y -
        m_Ball.radius -
        2.0f
    };

    InitializeLevel();

    m_State = State::Ready;

    m_PluginManager.NotifyGameStarted();
}

std::uint32_t Game::GetBrickCount() const
{
    return static_cast<std::uint32_t>(
        m_Bricks.size());
}

std::uint32_t Game::GetBrickColumns() const
{
    return m_BrickColumns;
}

std::uint32_t Game::GetBrickRows() const
{
    return m_BrickRows;
}

ArkanoidBrickInfo Game::MakeBrickInfo(
    const Brick& brick)
{
    ArkanoidBrickInfo info{};

    info.id = brick.id;

    info.row = brick.row;
    info.column = brick.column;
    info.typeId = brick.typeId;

    info.active =
        brick.active ? 1u : 0u;

    info.score =
        brick.score;

    info.red =
        brick.color.r;

    info.green =
        brick.color.g;

    info.blue =
        brick.color.b;

    return info;
}

bool Game::GetBrickInfo(
    const std::uint32_t index,
    ArkanoidBrickInfo& outBrick) const
{
    if (index >= m_Bricks.size())
        return false;

    outBrick =
        MakeBrickInfo(m_Bricks[index]);

    return true;
}

bool Game::GetBrickInfoAt(
    const std::uint32_t column,
    const std::uint32_t row,
    ArkanoidBrickInfo& outBrick) const
{
    if (column >= m_BrickColumns ||
        row >= m_BrickRows)
    {
        return false;
    }

    const std::uint32_t index =
        row * m_BrickColumns +
        column;

    return GetBrickInfo(
        index,
        outBrick);
}

void Game::SetBrickColor(
    const std::uint32_t brickId,
    const std::uint8_t red,
    const std::uint8_t green,
    const std::uint8_t blue)
{
    const auto it =
        std::find_if(
            m_Bricks.begin(),
            m_Bricks.end(),
            [brickId](const Brick& brick)
            {
                return brick.id == brickId;
            });

    if (it == m_Bricks.end())
        return;

    it->color.r = red;
    it->color.g = green;
    it->color.b = blue;
}

void Game::NormalizeBallVelocity()
{
    const float velocityLength =
        std::hypot(
            m_Ball.velocity.x,
            m_Ball.velocity.y);

    if (velocityLength <= 0.0f)
        return;

    m_Ball.velocity.x =
        m_Ball.velocity.x /
        velocityLength *
        m_Ball.speed;

    m_Ball.velocity.y =
        m_Ball.velocity.y /
        velocityLength *
        m_Ball.speed;
}

void Game::LoadPlugins(
    const std::filesystem::path& basePath)
{
    for (const GameConfig::Plugin& plugin :
         m_Config.plugins)
    {
        if (!plugin.enabled)
            continue;

        const std::filesystem::path path =
            basePath /
            plugin.library;

        if (!m_PluginManager.LoadPlugin(path))
        {
            SDL_Log(
                "Failed to load plugin: %s",
                path.string().c_str());
        }
        else
        {
            SDL_Log(
                "Loaded plugin: %s",
                path.string().c_str());
        }
    }

    m_PluginManager.NotifyGameStarted();
}
