#include "ArkanoidPlugin.h"

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

namespace
{
    constexpr float SpawnInterval = 8.0f;
    constexpr float SpecialDuration = 5.0f;

    constexpr float BlinkInterval = 0.20f;

    constexpr float BoostDuration = 5.0f;
    constexpr float BoostMultiplier = 1.5f;

    constexpr std::uint8_t HighlightRed = 255;
    constexpr std::uint8_t HighlightGreen = 220;
    constexpr std::uint8_t HighlightBlue = 40;

    struct PluginState
    {
        const ArkanoidHostApi* host = nullptr;

        std::mt19937 randomEngine{
            std::random_device{}()
        };

        float spawnTimer = SpawnInterval;

        bool specialActive = false;
        std::uint32_t specialBrickId = 0;

        std::uint8_t originalRed = 0;
        std::uint8_t originalGreen = 0;
        std::uint8_t originalBlue = 0;

        float specialTimeRemaining = 0.0f;

        float blinkTimer = 0.0f;
        bool highlightVisible = false;

        float boostTimeRemaining = 0.0f;
    };

    PluginState g_State;

    void SetBrickColor(
        PluginState& state,
        const std::uint32_t brickId,
        const std::uint8_t red,
        const std::uint8_t green,
        const std::uint8_t blue)
    {
        if (state.host == nullptr ||
            state.host->SetBrickColor == nullptr)
        {
            return;
        }

        state.host->SetBrickColor(
            state.host->userData,
            brickId,
            red,
            green,
            blue);
    }

    bool GetBrickInfo(
        PluginState& state,
        const std::uint32_t index,
        ArkanoidBrickInfo& outBrick)
    {
        if (state.host == nullptr ||
            state.host->GetBrickInfo == nullptr)
        {
            return false;
        }

        return state.host->GetBrickInfo(
            state.host->userData,
            index,
            &outBrick) != 0;
    }

    void RestoreSpecialBrickColor(
        PluginState& state)
    {
        if (!state.specialActive)
            return;

        SetBrickColor(
            state,
            state.specialBrickId,
            state.originalRed,
            state.originalGreen,
            state.originalBlue);
    }

    void ClearSpecialBrick(
        PluginState& state)
    {
        state.specialActive = false;

        state.specialTimeRemaining = 0.0f;
        state.blinkTimer = 0.0f;
        state.highlightVisible = false;
    }

    bool SelectRandomActiveBrick(
        PluginState& state)
    {
        if (state.host == nullptr ||
            state.host->GetBrickCount == nullptr)
        {
            return false;
        }

        const std::uint32_t brickCount =
            state.host->GetBrickCount(
                state.host->userData);

        std::vector<ArkanoidBrickInfo> candidates;

        candidates.reserve(brickCount);

        for (std::uint32_t index = 0;
             index < brickCount;
             ++index)
        {
            ArkanoidBrickInfo brick{};

            if (!GetBrickInfo(
                    state,
                    index,
                    brick))
            {
                continue;
            }

            if (brick.active == 0)
                continue;

            candidates.push_back(brick);
        }

        if (candidates.empty())
            return false;

        std::uniform_int_distribution<std::size_t>
            distribution(
                0,
                candidates.size() - 1);

        const ArkanoidBrickInfo& brick =
            candidates[
                distribution(
                    state.randomEngine)];

        state.specialBrickId =
            brick.id;

        state.originalRed =
            brick.red;

        state.originalGreen =
            brick.green;

        state.originalBlue =
            brick.blue;

        state.specialActive = true;

        state.specialTimeRemaining =
            SpecialDuration;

        state.blinkTimer = 0.0f;

        state.highlightVisible = true;

        SetBrickColor(
            state,
            brick.id,
            HighlightRed,
            HighlightGreen,
            HighlightBlue);

        return true;
    }

    void OnLoad(
        void* userData,
        const ArkanoidHostApi* host)
    {
        if (userData == nullptr)
            return;

        auto* state =
            static_cast<PluginState*>(
                userData);

        state->host = host;
    }

    void OnUnload(
        void* userData)
    {
        if (userData == nullptr)
            return;

        auto* state =
            static_cast<PluginState*>(
                userData);

        RestoreSpecialBrickColor(
            *state);

        state->host = nullptr;
    }

    void OnGameStarted(
        void* userData)
    {
        if (userData == nullptr)
            return;

        auto* state =
            static_cast<PluginState*>(
                userData);

        state->spawnTimer =
            SpawnInterval;

        state->specialActive =
            false;

        state->specialTimeRemaining =
            0.0f;

        state->blinkTimer =
            0.0f;

        state->highlightVisible =
            false;

        state->boostTimeRemaining =
            0.0f;
    }

    void OnGameEnded(
        void* userData,
        ArkanoidGameResult)
    {
        if (userData == nullptr)
            return;

        auto* state =
            static_cast<PluginState*>(
                userData);

        RestoreSpecialBrickColor(
            *state);

        ClearSpecialBrick(
            *state);

        state->boostTimeRemaining =
            0.0f;
    }

    void ModifyBallUpdate(
        void* userData,
        ArkanoidBallUpdateContext* context)
    {
        if (userData == nullptr ||
            context == nullptr)
        {
            return;
        }

        auto* state =
            static_cast<PluginState*>(
                userData);

        if (state->boostTimeRemaining > 0.0f)
        {
            context->speedMultiplier *=
                BoostMultiplier;
        }
    }

    void OnFixedUpdate(
        void* userData,
        const float deltaTime)
    {
        if (userData == nullptr)
            return;

        auto* state =
            static_cast<PluginState*>(
                userData);

        //
        // Ball boost timer
        //

        if (state->boostTimeRemaining > 0.0f)
        {
            state->boostTimeRemaining =
                std::max(
                    0.0f,
                    state->boostTimeRemaining -
                        deltaTime);
        }

        //
        // Active special brick
        //

        if (state->specialActive)
        {
            state->specialTimeRemaining -=
                deltaTime;

            state->blinkTimer -=
                deltaTime;

            if (state->specialTimeRemaining <= 0.0f)
            {
                RestoreSpecialBrickColor(
                    *state);

                ClearSpecialBrick(
                    *state);

                state->spawnTimer =
                    SpawnInterval;

                return;
            }

            if (state->blinkTimer <= 0.0f)
            {
                state->blinkTimer +=
                    BlinkInterval;

                state->highlightVisible =
                    !state->highlightVisible;

                if (state->highlightVisible)
                {
                    SetBrickColor(
                        *state,
                        state->specialBrickId,
                        HighlightRed,
                        HighlightGreen,
                        HighlightBlue);
                }
                else
                {
                    SetBrickColor(
                        *state,
                        state->specialBrickId,
                        state->originalRed,
                        state->originalGreen,
                        state->originalBlue);
                }
            }

            return;
        }

        //
        // Wait before selecting another special brick.
        //

        state->spawnTimer -=
            deltaTime;

        if (state->spawnTimer > 0.0f)
            return;

        if (!SelectRandomActiveBrick(
                *state))
        {
            state->spawnTimer =
                SpawnInterval;

            return;
        }
    }

    void OnBrickDestroyed(
        void* userData,
        const ArkanoidBrickDestroyedEvent* event)
    {
        if (userData == nullptr ||
            event == nullptr)
        {
            return;
        }

        auto* state =
            static_cast<PluginState*>(
                userData);

        if (!state->specialActive)
            return;

        if (event->brick.id !=
            state->specialBrickId)
        {
            return;
        }

        //
        // The special brick was destroyed in time.
        //

        state->boostTimeRemaining =
            BoostDuration;

        ClearSpecialBrick(
            *state);

        state->spawnTimer =
            SpawnInterval;
    }
    
    void ModifyBrickHit(
    void* userData,
    ArkanoidBrickHitContext* context)
    {
        if (userData == nullptr ||
            context == nullptr)
        {
            return;
        }

        auto* state =
            static_cast<PluginState*>(
                userData);

        if (!state->specialActive)
            return;

        if (context->brick.id ==
            state->specialBrickId)
        {
            context->score *= 5;
        }
    }

    ArkanoidPluginApi g_PluginApi =
    {
        .apiVersion =
            ARKANOID_PLUGIN_API_VERSION,

        .structSize =
            sizeof(ArkanoidPluginApi),

        .name =
            "Bonus Brick",

        .userData =
            &g_State,

        .OnLoad =
            OnLoad,

        .OnUnload =
            OnUnload,

        .OnGameStarted =
            OnGameStarted,

        .OnGameEnded =
            OnGameEnded,

        .ModifyPaddleUpdate =
            nullptr,

        .ModifyBallUpdate =
            ModifyBallUpdate,

        .ModifyPaddleReflection =
            nullptr,

        .ModifyBrickHit =
            ModifyBrickHit,

        .OnFixedUpdate =
            OnFixedUpdate,

        .OnBrickDestroyed =
            OnBrickDestroyed
    };
}

ARKANOID_PLUGIN_EXPORT
ArkanoidPluginApi* ArkanoidGetPluginApi()
{
    return &g_PluginApi;
}