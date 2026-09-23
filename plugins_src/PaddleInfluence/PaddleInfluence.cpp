#include "ArkanoidPlugin.h"

#include <cmath>

namespace
{
    struct PluginState
    {
        float influence = 0.35f;
    };

    PluginState g_State;

    void ModifyPaddleReflection(
        void* userData,
        ArkanoidPaddleReflectionContext* context)
    {
        if (userData == nullptr ||
            context == nullptr)
        {
            return;
        }

        auto* state =
            static_cast<PluginState*>(userData);

        //
        // Preserve the speed of the geometrically reflected ball.
        //

        const float speed =
            std::hypot(
                context->velocityX,
                context->velocityY);

        if (speed <= 0.0f)
            return;

        //
        // Add horizontal influence from the moving paddle.
        //

        context->velocityX +=
            context->paddleVelocityX *
            state->influence;

        //
        // Keep the original ball speed.
        //
        // The plugin changes direction only.
        //

        const float modifiedSpeed =
            std::hypot(
                context->velocityX,
                context->velocityY);

        if (modifiedSpeed <= 0.0f)
            return;

        context->velocityX =
            context->velocityX /
            modifiedSpeed *
            speed;

        context->velocityY =
            context->velocityY /
            modifiedSpeed *
            speed;
    }

    ArkanoidPluginApi g_PluginApi =
    {
        .apiVersion =
            ARKANOID_PLUGIN_API_VERSION,

        .structSize =
            sizeof(ArkanoidPluginApi),

        .name =
            "Paddle Influence",

        .userData =
            &g_State,

        .OnLoad =
            nullptr,

        .OnUnload =
            nullptr,

        .OnGameStarted =
            nullptr,

        .OnGameEnded =
            nullptr,

        .ModifyPaddleUpdate =
            nullptr,

        .ModifyBallUpdate =
            nullptr,

        .ModifyPaddleReflection =
            ModifyPaddleReflection,

        .ModifyBrickHit =
            nullptr,

        .OnFixedUpdate =
            nullptr,

        .OnBrickDestroyed =
            nullptr
    };
}

ARKANOID_PLUGIN_EXPORT
ArkanoidPluginApi* ArkanoidGetPluginApi()
{
    return &g_PluginApi;
}