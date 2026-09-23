#include "ConfigLoader.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace
{
    void LoadWindow(
        const json& root,
        GameConfig::Window& config)
    {
        if (!root.contains("window"))
            return;

        const json& data = root["window"];

        config.width =
            data.value("width", config.width);

        config.height =
            data.value("height", config.height);
    }

    void LoadPaddle(
        const json& root,
        GameConfig::Paddle& config)
    {
        if (!root.contains("paddle"))
            return;

        const json& data = root["paddle"];

        config.width =
            data.value("width", config.width);

        config.height =
            data.value("height", config.height);

        config.speed =
            data.value("speed", config.speed);

        config.bottomOffset =
            data.value(
                "bottomOffset",
                config.bottomOffset);

        config.influence =
            data.value(
                "influence",
                config.influence);
    }

    void LoadBall(
        const json& root,
        GameConfig::Ball& config)
    {
        if (!root.contains("ball"))
            return;

        const json& data = root["ball"];

        config.radius =
            data.value("radius", config.radius);

        config.speed =
            data.value("speed", config.speed);

        config.count =
            data.value("count", config.count);
    }

    void LoadBrickType(
        const json& data,
        GameConfig::BrickType& config)
    {
        config.rows =
            data.value("rows", config.rows);

        config.score =
            data.value("score", config.score);

        if (data.contains("color"))
        {
            const auto& color = data["color"];

            if (color.is_array() &&
                color.size() >= 3)
            {
                config.red =
                    color[0].get<std::uint8_t>();

                config.green =
                    color[1].get<std::uint8_t>();

                config.blue =
                    color[2].get<std::uint8_t>();
            }
        }
    }

    void LoadBricks(
        const json& root,
        GameConfig::Bricks& config)
    {
        if (!root.contains("bricks"))
            return;

        const json& data = root["bricks"];

        config.columns =
            data.value(
                "columns",
                config.columns);

        config.width =
            data.value(
                "width",
                config.width);

        config.height =
            data.value(
                "height",
                config.height);

        config.spacing =
            data.value(
                "spacing",
                config.spacing);

        config.topOffset =
            data.value(
                "topOffset",
                config.topOffset);

        if (!data.contains("types"))
            return;

        const json& types = data["types"];

        if (!types.is_array())
            return;

        config.types.clear();

        for (const json& item : types)
        {
            GameConfig::BrickType type;

            LoadBrickType(item, type);

            config.types.push_back(type);
        }
    }

    void LoadPhysics(
        const json& root,
        GameConfig::Physics& config)
    {
        if (!root.contains("physics"))
            return;

        const json& data = root["physics"];

        config.fixedUpdateRate =
            data.value(
                "fixedUpdateRate",
                config.fixedUpdateRate);
    }

    void LoadPlugins(
        const json& root,
        GameConfig& config)
    {
        if (!root.contains("plugins"))
            return;

        const json& plugins =
            root["plugins"];

        if (!plugins.is_array())
            return;

        config.plugins.clear();

        for (const json& item : plugins)
        {
            if (!item.is_object())
                continue;

            GameConfig::Plugin plugin;

            plugin.library =
                item.value(
                    "library",
                    std::string{});

            plugin.enabled =
                item.value(
                    "enabled",
                    true);

            if (!plugin.library.empty())
            {
                config.plugins.push_back(
                    std::move(plugin));
            }
        }
    }
}

GameConfig ConfigLoader::Load(
    const std::filesystem::path& path)
{
    GameConfig config;

    std::ifstream stream(path);

    if (!stream.is_open())
    {
        return config;
    }

    json root;

    try
    {
        stream >> root;

        LoadWindow(root, config.window);
        LoadPaddle(root, config.paddle);
        LoadBall(root, config.ball);
        LoadBricks(root, config.bricks);
        LoadPhysics(root, config.physics);
        LoadPlugins(root, config);
    }
    catch (const json::exception&)
    {
        return GameConfig{};
    }

    return config;
}
