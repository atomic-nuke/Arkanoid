#include <SDL3/SDL.h>

#include "Game.h"
#include "ConfigLoader.h"

#include <chrono>

int main(int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log(
            "SDL initialization failed: %s",
            SDL_GetError()
        );

        return 1;
    }

    const std::filesystem::path basePath =
        SDL_GetBasePath();

    const GameConfig config =
        ConfigLoader::Load(basePath / "config.json");


    SDL_Window* window = SDL_CreateWindow(
        "Arkanoid",
        config.window.width,
        config.window.height,
        0);

    if (window == nullptr)
    {
        SDL_Log(
            "Window creation failed: %s",
            SDL_GetError()
        );

        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, nullptr);

    if (renderer == nullptr)
    {
        SDL_Log(
            "Renderer creation failed: %s",
            SDL_GetError()
        );

        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }

    Game game(
        renderer,
        config);

    game.LoadPlugins(
        basePath);

    using Clock = std::chrono::steady_clock;

    const float fixedUpdateRate =
        std::max(
            config.physics.fixedUpdateRate,
            1.0f);

    const float fixedDeltaTime =
        1.0f / fixedUpdateRate;

    auto previousTime = Clock::now();

    float accumulator = 0.0f;

    while (game.IsRunning())
    {
        const auto currentTime = Clock::now();

        const std::chrono::duration<float> elapsed =
            currentTime - previousTime;

        previousTime = currentTime;

        float frameTime = elapsed.count();

        // Skip big steps
        frameTime = std::min(frameTime, 0.25f);

        accumulator += frameTime;

        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            game.ProcessEvent(event);
        }

        while (accumulator >= fixedDeltaTime)
        {
            game.Update(fixedDeltaTime);
            accumulator -= fixedDeltaTime;
        }

        game.Render();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
