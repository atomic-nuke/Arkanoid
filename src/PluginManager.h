#pragma once

#include "ArkanoidPlugin.h"

#include <cstdint>
#include <vector>
#include <filesystem>

class Game;

class PluginManager
{
public:
    explicit PluginManager(Game& game);
    ~PluginManager();

    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;

    bool LoadPlugin(const std::filesystem::path& path);
    void NotifyGameStarted();
    void QueueGameEnded(ArkanoidGameResult result);

public:
    // Plugin lifetime

    bool RegisterPlugin(ArkanoidPluginApi* plugin);

    void UnregisterAll();

    // Modifier callbacks

    void ModifyPaddleUpdate(
        ArkanoidPaddleUpdateContext& context);

    void ModifyBallUpdate(
        ArkanoidBallUpdateContext& context);

    void ModifyPaddleReflection(
        ArkanoidPaddleReflectionContext& context);

    void ModifyBrickHit(
        ArkanoidBrickHitContext& context);


    // Plugin update

    void FixedUpdate(float deltaTime);

    // Events

    void QueueBrickDestroyed(
        const ArkanoidBrickDestroyedEvent& event);

    void DispatchQueuedEvents();
    
    // Deferred host commands

    void ApplyQueuedCommands();

private:
    struct PluginEntry
    {
        ArkanoidPluginApi* api = nullptr;

        void* libraryHandle = nullptr;
    };

    struct HostContext
    {
        Game* game = nullptr;
        PluginManager* pluginManager = nullptr;
    };

    struct BrickColorCommand
    {
        std::uint32_t brickId = 0;

        std::uint8_t red = 0;
        std::uint8_t green = 0;
        std::uint8_t blue = 0;
    };

    struct QueuedGameEndedEvent
    {
        bool pending = false;
        ArkanoidGameResult result =
            ARKANOID_GAME_OVER;
    };

private:
    // Host API callbacks

    static std::uint32_t HostGetBrickCount(
        void* userData);

    static std::uint32_t HostGetBrickColumns(
        void* userData);

    static std::uint32_t HostGetBrickRows(
        void* userData);

    static std::uint32_t HostGetBrickInfo(
        void* userData,
        std::uint32_t index,
        ArkanoidBrickInfo* outBrick);

    static std::uint32_t HostGetBrickInfoAt(
        void* userData,
        std::uint32_t column,
        std::uint32_t row,
        ArkanoidBrickInfo* outBrick);

    static void HostSetBrickColor(
        void* userData,
        std::uint32_t brickId,
        std::uint8_t red,
        std::uint8_t green,
        std::uint8_t blue);

private:
    HostContext m_HostContext{};
    ArkanoidHostApi m_HostApi{};

    std::vector<PluginEntry> m_Plugins;

    std::vector<ArkanoidBrickDestroyedEvent>
    m_BrickDestroyedEvents;

    std::vector<BrickColorCommand>
    m_BrickColorCommands;

    QueuedGameEndedEvent
    m_GameEndedEvent{};
};
