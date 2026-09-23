#include "PluginManager.h"

#include "Game.h"

#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#endif

PluginManager::PluginManager(Game& game)
{
    m_HostContext.game =
        &game;

    m_HostContext.pluginManager =
        this;

    m_HostApi.apiVersion =
        ARKANOID_PLUGIN_API_VERSION;

    m_HostApi.structSize =
        sizeof(ArkanoidHostApi);

    m_HostApi.userData =
        &m_HostContext;

    m_HostApi.GetBrickCount =
        &PluginManager::HostGetBrickCount;

    m_HostApi.GetBrickColumns =
        &PluginManager::HostGetBrickColumns;

    m_HostApi.GetBrickRows =
        &PluginManager::HostGetBrickRows;

    m_HostApi.GetBrickInfo =
        &PluginManager::HostGetBrickInfo;

    m_HostApi.GetBrickInfoAt =
        &PluginManager::HostGetBrickInfoAt;

    m_HostApi.SetBrickColor =
        &PluginManager::HostSetBrickColor;
}

PluginManager::~PluginManager()
{
    UnregisterAll();
}

bool PluginManager::RegisterPlugin(
    ArkanoidPluginApi* plugin)
{
    if (plugin == nullptr)
        return false;

    if (plugin->apiVersion !=
        ARKANOID_PLUGIN_API_VERSION)
    {
        return false;
    }

    if (plugin->structSize <
        sizeof(ArkanoidPluginApi))
    {
        return false;
    }

    PluginEntry entry;
    entry.api = plugin;

    m_Plugins.push_back(entry);

    if (plugin->OnLoad != nullptr)
    {
        plugin->OnLoad(
            plugin->userData,
            &m_HostApi);
    }

    return true;
}

void PluginManager::UnregisterAll()
{
    for (PluginEntry& entry :
         m_Plugins)
    {
        ArkanoidPluginApi* plugin =
            entry.api;

        if (plugin != nullptr &&
            plugin->OnUnload != nullptr)
        {
            plugin->OnUnload(
                plugin->userData);
        }

#ifdef _WIN32
        if (entry.libraryHandle != nullptr)
        {
            FreeLibrary(
                static_cast<HMODULE>(
                    entry.libraryHandle));
        }
#endif
    }

    m_Plugins.clear();

    m_BrickDestroyedEvents.clear();
    m_BrickColorCommands.clear();

    m_GameEndedEvent = {};
}

void PluginManager::ModifyPaddleUpdate(
    ArkanoidPaddleUpdateContext& context)
{
    for (const PluginEntry& entry : m_Plugins)
    {
        ArkanoidPluginApi* plugin =
            entry.api;

        if (plugin == nullptr ||
            plugin->ModifyPaddleUpdate == nullptr)
        {
            continue;
        }

        plugin->ModifyPaddleUpdate(
            plugin->userData,
            &context);
    }
}

void PluginManager::ModifyBallUpdate(
    ArkanoidBallUpdateContext& context)
{
    for (const PluginEntry& entry : m_Plugins)
    {
        ArkanoidPluginApi* plugin =
            entry.api;

        if (plugin == nullptr ||
            plugin->ModifyBallUpdate == nullptr)
        {
            continue;
        }

        plugin->ModifyBallUpdate(
            plugin->userData,
            &context);
    }
}

void PluginManager::ModifyPaddleReflection(
    ArkanoidPaddleReflectionContext& context)
{
    for (const PluginEntry& entry : m_Plugins)
    {
        ArkanoidPluginApi* plugin =
            entry.api;

        if (plugin == nullptr ||
            plugin->ModifyPaddleReflection == nullptr)
        {
            continue;
        }

        plugin->ModifyPaddleReflection(
            plugin->userData,
            &context);
    }
}

void PluginManager::ModifyBrickHit(
    ArkanoidBrickHitContext& context)
{
    for (const PluginEntry& entry : m_Plugins)
    {
        ArkanoidPluginApi* plugin =
            entry.api;

        if (plugin == nullptr ||
            plugin->ModifyBrickHit == nullptr)
        {
            continue;
        }

        plugin->ModifyBrickHit(
            plugin->userData,
            &context);
    }
}

void PluginManager::FixedUpdate(
    const float deltaTime)
{
    for (const PluginEntry& entry : m_Plugins)
    {
        ArkanoidPluginApi* plugin =
            entry.api;

        if (plugin == nullptr ||
            plugin->OnFixedUpdate == nullptr)
        {
            continue;
        }

        plugin->OnFixedUpdate(
            plugin->userData,
            deltaTime);
    }
}

void PluginManager::QueueBrickDestroyed(
    const ArkanoidBrickDestroyedEvent& event)
{
    m_BrickDestroyedEvents.push_back(event);
}

void PluginManager::DispatchQueuedEvents()
{
    for (const ArkanoidBrickDestroyedEvent& event :
         m_BrickDestroyedEvents)
    {
        for (const PluginEntry& entry : m_Plugins)
        {
            ArkanoidPluginApi* plugin =
                entry.api;

            if (plugin == nullptr ||
                plugin->OnBrickDestroyed == nullptr)
            {
                continue;
            }

            plugin->OnBrickDestroyed(
                plugin->userData,
                &event);
        }
    }

    m_BrickDestroyedEvents.clear();

    if (m_GameEndedEvent.pending)
    {
        for (const PluginEntry& entry :
             m_Plugins)
        {
            ArkanoidPluginApi* plugin =
                entry.api;

            if (plugin == nullptr ||
                plugin->OnGameEnded == nullptr)
            {
                continue;
            }

            plugin->OnGameEnded(
                plugin->userData,
                m_GameEndedEvent.result);
        }

        m_GameEndedEvent.pending =
            false;
    }
}

std::uint32_t PluginManager::HostGetBrickCount(
    void* userData)
{
    auto* context =
        static_cast<HostContext*>(
            userData);

    if (context == nullptr ||
        context->game == nullptr)
    {
        return 0;
    }

    return context->game->GetBrickCount();
}

std::uint32_t PluginManager::HostGetBrickColumns(
    void* userData)
{
    auto* context =
        static_cast<HostContext*>(
            userData);

    if (context == nullptr ||
        context->game == nullptr)
    {
        return 0;
    }

    return context->game->GetBrickColumns();
}

std::uint32_t PluginManager::HostGetBrickRows(
    void* userData)
{
    auto* context =
        static_cast<HostContext*>(
            userData);

    if (context == nullptr ||
        context->game == nullptr)
    {
        return 0;
    }

    return context->game->GetBrickRows();
}

std::uint32_t PluginManager::HostGetBrickInfo(
    void* userData,
    const std::uint32_t index,
    ArkanoidBrickInfo* outBrick)
{
    if (outBrick == nullptr)
        return 0;

    auto* context =
        static_cast<HostContext*>(
            userData);

    if (context == nullptr ||
        context->game == nullptr)
    {
        return 0;
    }

    return context->game->GetBrickInfo(
               index,
               *outBrick)
               ? 1u
               : 0u;
}

std::uint32_t PluginManager::HostGetBrickInfoAt(
    void* userData,
    const std::uint32_t column,
    const std::uint32_t row,
    ArkanoidBrickInfo* outBrick)
{
    if (outBrick == nullptr)
        return 0;

    auto* context =
        static_cast<HostContext*>(
            userData);

    if (context == nullptr ||
        context->game == nullptr)
    {
        return 0;
    }

    return context->game->GetBrickInfoAt(
               column,
               row,
               *outBrick)
               ? 1u
               : 0u;
}

void PluginManager::HostSetBrickColor(
    void* userData,
    const std::uint32_t brickId,
    const std::uint8_t red,
    const std::uint8_t green,
    const std::uint8_t blue)
{
    auto* context =
        static_cast<HostContext*>(
            userData);

    if (context == nullptr ||
        context->pluginManager == nullptr)
    {
        return;
    }

    BrickColorCommand command;

    command.brickId = brickId;

    command.red = red;
    command.green = green;
    command.blue = blue;

    context->pluginManager
           ->m_BrickColorCommands
           .push_back(command);
}

void PluginManager::ApplyQueuedCommands()
{
    for (const BrickColorCommand& command :
         m_BrickColorCommands)
    {
        if (m_HostContext.game == nullptr)
            break;

        m_HostContext.game->SetBrickColor(
            command.brickId,
            command.red,
            command.green,
            command.blue);
    }

    m_BrickColorCommands.clear();
}

bool PluginManager::LoadPlugin(
    const std::filesystem::path& path)
{
#ifdef _WIN32

    HMODULE library =
        LoadLibraryW(
            path.c_str());

    if (library == nullptr)
    {
        return false;
    }

    FARPROC procedure =
        GetProcAddress(
            library,
            ARKANOID_PLUGIN_ENTRY_POINT);

    if (procedure == nullptr)
    {
        FreeLibrary(library);
        return false;
    }

    const auto getPluginApi =
        reinterpret_cast<
            ArkanoidGetPluginApiFunction>(
            procedure);

    ArkanoidPluginApi* plugin =
        getPluginApi();

    if (plugin == nullptr)
    {
        FreeLibrary(library);
        return false;
    }

    if (plugin->apiVersion !=
        ARKANOID_PLUGIN_API_VERSION)
    {
        FreeLibrary(library);
        return false;
    }

    if (plugin->structSize <
        sizeof(ArkanoidPluginApi))
    {
        FreeLibrary(library);
        return false;
    }

    PluginEntry entry;

    entry.api =
        plugin;

    entry.libraryHandle =
        library;

    m_Plugins.push_back(entry);

    if (plugin->OnLoad != nullptr)
    {
        plugin->OnLoad(
            plugin->userData,
            &m_HostApi);
    }

    return true;

#else

    return false;

#endif
}

void PluginManager::NotifyGameStarted()
{
    for (const PluginEntry& entry :
         m_Plugins)
    {
        ArkanoidPluginApi* plugin =
            entry.api;

        if (plugin == nullptr ||
            plugin->OnGameStarted == nullptr)
        {
            continue;
        }

        plugin->OnGameStarted(
            plugin->userData);
    }
}

void PluginManager::QueueGameEnded(
    const ArkanoidGameResult result)
{
    m_GameEndedEvent.pending =
        true;

    m_GameEndedEvent.result =
        result;
}
