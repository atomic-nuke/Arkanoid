#pragma once

#include "GameConfig.h"

#include <filesystem>

class ConfigLoader
{
public:
    [[nodiscard]]
    static GameConfig Load(
        const std::filesystem::path& path);
};
