# Arkanoid

A small Arkanoid-style game written in C++ using SDL3.

The project focuses primarily on code structure, deterministic fixed-step gameplay and a small runtime DLL plugin system.

## Features

- Paddle movement using keyboard input
- Ball launching with `Space`
- Brick grid with multiple brick types and different score values
- Score and remaining balls displayed in-game
- Win and game-over states
- JSON-based configuration
- Fixed timestep simulation
- Runtime DLL plugin system with a small C ABI
- Included example plugins:
  - `PaddleInfluence` - modifies ball reflection based on paddle movement
  - `BonusBrick` - periodically marks a bonus brick and can apply score/speed bonuses

## Controls

- `Left Arrow` / `Right Arrow` - move paddle
- `Space` - launch ball
- `R` - restart after win or game over
- `Escape` - quit

## Requirements

- Windows
- CMake
- C++20 compatible compiler
- Git

## Clone

The project uses Git submodules for SDL3 and nlohmann/json.

Clone recursively:

```bash
git clone --recursive <repository-url>
cd Arkanoid
```

If the repository was already cloned without submodules:

```bash
git submodule update --init --recursive
```

## Build

### Command line

Configure:

```bash
cmake -S . -B build
```

Build:

```bash
cmake --build build --config Debug
```

The exact output directory depends on the selected CMake generator.

## Configuration

Gameplay settings are stored in:

```text
config.json
```

The configuration includes:

- window size
- paddle parameters
- ball parameters
- brick layout and score values
- fixed update rate
- enabled runtime plugins

Example:

```json
"plugins": [
  {
    "library": "plugins/PaddleInfluence.dll",
    "enabled": true
  },
  {
    "library": "plugins/BonusBrick.dll",
    "enabled": true
  }
]
```

Plugins are loaded in the order in which they appear in the configuration.

`plugin_sdk/ArkanoidPlugin.h` contains the public C ABI shared between the host application and runtime plugins.

The game itself does not depend on the implementation details of individual plugins.
