#pragma once

#include <cstdint>
#include <vector>
#include <string>

struct GameConfig
{
    struct Window
    {
        int width = 1280;
        int height = 720;
    };

    struct Paddle
    {
        float width = 160.0f;
        float height = 20.0f;
        float speed = 700.0f;
        float bottomOffset = 50.0f;
        float influence = 0.35f;
    };

    struct Ball
    {
        float radius = 8.0f;
        float speed = 450.0f;
        int count = 3;
    };

    struct BrickType
    {
        int rows = 1;
        int score = 10;

        std::uint8_t red = 255;
        std::uint8_t green = 255;
        std::uint8_t blue = 255;
    };

    struct Bricks
    {
        int columns = 10;

        float width = 100.0f;
        float height = 30.0f;
        float spacing = 8.0f;
        float topOffset = 80.0f;

        std::vector<BrickType> types =
        {
            {
                2,
                20,
                220,
                70,
                70
            },
            {
                3,
                10,
                70,
                140,
                220
            }
        };
    };

    struct Physics
    {
        float fixedUpdateRate = 120.0f;
    };

    struct Plugin
    {
        std::string library;
        bool enabled = true;
    };

    Window window;
    Paddle paddle;
    Ball ball;
    Bricks bricks;
    Physics physics;
    std::vector<Plugin> plugins;
};
