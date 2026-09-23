#pragma once

#include <stddef.h>
#include <stdint.h>

#define ARKANOID_PLUGIN_API_VERSION 1

#ifdef __cplusplus
    #define ARKANOID_EXTERN_C extern "C"
#else
    #define ARKANOID_EXTERN_C
#endif

#ifdef _WIN32
    #define ARKANOID_PLUGIN_EXPORT \
        ARKANOID_EXTERN_C __declspec(dllexport)
#else
    #define ARKANOID_PLUGIN_EXPORT \
        ARKANOID_EXTERN_C
#endif

#define ARKANOID_PLUGIN_ENTRY_POINT \
    "ArkanoidGetPluginApi"

// Game state

typedef enum ArkanoidGameResult
{
    ARKANOID_GAME_WON = 0,
    ARKANOID_GAME_OVER = 1

} ArkanoidGameResult;

// Brick information

typedef struct ArkanoidBrickInfo
{
    uint32_t id;

    uint32_t row;
    uint32_t column;
    uint32_t typeId;

    // 0 = inactive, non-zero = active.
    uint32_t active;

    int32_t score;

    uint8_t red;
    uint8_t green;
    uint8_t blue;

} ArkanoidBrickInfo;

// Modifier contexts

typedef struct ArkanoidPaddleUpdateContext
{
    float deltaTime;

    float speedMultiplier;
    float widthMultiplier;

} ArkanoidPaddleUpdateContext;

typedef struct ArkanoidBallUpdateContext
{
    float deltaTime;

    float speedMultiplier;

} ArkanoidBallUpdateContext;

typedef struct ArkanoidPaddleReflectionContext
{
    float velocityX;
    float velocityY;

    float paddleVelocityX;

} ArkanoidPaddleReflectionContext;

typedef struct ArkanoidBrickHitContext
{
    ArkanoidBrickInfo brick;

    int32_t baseScore;
    int32_t score;

    float velocityX;
    float velocityY;

} ArkanoidBrickHitContext;

// Events

typedef struct ArkanoidBrickDestroyedEvent
{
    ArkanoidBrickInfo brick;

    int32_t awardedScore;

} ArkanoidBrickDestroyedEvent;

// API provided by Arkanoid to plugins

typedef struct ArkanoidHostApi
{
    uint32_t apiVersion;

    // Size of this structure for ABI compatibility checks.
    uint32_t structSize;

    // Opaque host-owned pointer passed back to host callbacks.
    void* userData;

    uint32_t (*GetBrickCount)(
        void* userData);

    uint32_t (*GetBrickColumns)(
        void* userData);

    uint32_t (*GetBrickRows)(
        void* userData);

    uint32_t (*GetBrickInfo)(
        void* userData,
        uint32_t index,
        ArkanoidBrickInfo* outBrick);

    uint32_t (*GetBrickInfoAt)(
        void* userData,
        uint32_t column,
        uint32_t row,
        ArkanoidBrickInfo* outBrick);

    void (*SetBrickColor)(
        void* userData,
        uint32_t brickId,
        uint8_t red,
        uint8_t green,
        uint8_t blue);

} ArkanoidHostApi;

// API provided by plugins to Arkanoid

typedef struct ArkanoidPluginApi
{
    uint32_t apiVersion;

    // Size of this structure for ABI compatibility checks.
    uint32_t structSize;

    const char* name;

    // Opaque plugin-owned pointer passed back to plugin callbacks.
    void* userData;

    void (*OnLoad)(
        void* userData,
        const ArkanoidHostApi* host);

    void (*OnUnload)(
        void* userData);

    void (*OnGameStarted)(
        void* userData);

    void (*OnGameEnded)(
        void* userData,
        ArkanoidGameResult result);

    void (*ModifyPaddleUpdate)(
        void* userData,
        ArkanoidPaddleUpdateContext* context);

    void (*ModifyBallUpdate)(
        void* userData,
        ArkanoidBallUpdateContext* context);

    void (*ModifyPaddleReflection)(
        void* userData,
        ArkanoidPaddleReflectionContext* context);

    void (*ModifyBrickHit)(
        void* userData,
        ArkanoidBrickHitContext* context);

    void (*OnFixedUpdate)(
        void* userData,
        float deltaTime);

    void (*OnBrickDestroyed)(
        void* userData,
        const ArkanoidBrickDestroyedEvent* event);

} ArkanoidPluginApi;

// Plugin DLL entry point

typedef ArkanoidPluginApi*
    (*ArkanoidGetPluginApiFunction)();

ARKANOID_PLUGIN_EXPORT
ArkanoidPluginApi* ArkanoidGetPluginApi();