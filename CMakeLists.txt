cmake_minimum_required(VERSION 3.25)

project(Arkanoid
        VERSION 1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
add_subdirectory(external/json EXCLUDE_FROM_ALL)

add_executable(Arkanoid
        src/main.cpp

        src/Game.cpp
        src/Game.h

        src/GameConfig.h

        src/ConfigLoader.cpp
        src/ConfigLoader.h

        src/PluginManager.cpp
        src/PluginManager.h

        plugin_sdk/ArkanoidPlugin.h
)

target_link_libraries(Arkanoid
        PRIVATE
        SDL3::SDL3
        nlohmann_json::nlohmann_json
)

target_include_directories(Arkanoid
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/plugin_sdk"
)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
        "${CMAKE_CURRENT_BINARY_DIR}/config.json"
        COPYONLY
)

add_custom_command(
        TARGET Arkanoid POST_BUILD

        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/config.json"
        "$<TARGET_FILE_DIR:Arkanoid>/config.json"

        VERBATIM
)

if (WIN32)
    add_custom_command(
            TARGET Arkanoid POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE_DIR:Arkanoid>
            VERBATIM
    )
endif ()

add_library(PaddleInfluence SHARED
        plugins_src/PaddleInfluence/PaddleInfluence.cpp
        plugin_sdk/ArkanoidPlugin.h
)

target_include_directories(PaddleInfluence
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/plugin_sdk"
)

add_custom_command(
        TARGET PaddleInfluence POST_BUILD

        COMMAND "${CMAKE_COMMAND}" -E make_directory
        "$<TARGET_FILE_DIR:Arkanoid>/plugins"

        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_FILE:PaddleInfluence>"
        "$<TARGET_FILE_DIR:Arkanoid>/plugins/$<TARGET_FILE_NAME:PaddleInfluence>"

        VERBATIM
)


add_library(BonusBrick SHARED
        plugins_src/BonusBrick/BonusBrick.cpp
        plugin_sdk/ArkanoidPlugin.h
)

target_include_directories(BonusBrick
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/plugin_sdk"
)

add_custom_command(
        TARGET BonusBrick POST_BUILD

        COMMAND "${CMAKE_COMMAND}" -E make_directory
        "$<TARGET_FILE_DIR:Arkanoid>/plugins"

        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_FILE:BonusBrick>"
        "$<TARGET_FILE_DIR:Arkanoid>/plugins/$<TARGET_FILE_NAME:BonusBrick>"

        VERBATIM
)

add_dependencies(
        Arkanoid
        PaddleInfluence
        BonusBrick
)