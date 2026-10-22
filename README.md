# Baseball Arcade - raylib / C99 / CMake / Visual Studio 2019

A screenshot-inspired pixel-art baseball batting screen implemented entirely with raylib primitives.

## Features

- C99
- raylib 5.5 fetched by CMake
- Visual Studio 2019 generator support
- 993x728 window matching the supplied reference aspect ratio
- Pixel-art style pitcher, batter, baseball field, bases, foul lines and HUD
- Functional pitch animation
- Functional swing timing
- Ball/strike/outs scoreboard
- R to reset
- Space, Z or Enter to swing
- No external art assets required

## Build with Visual Studio 2019

From an x64 Native Tools Command Prompt for VS 2019:

```bat
cmake -S . -B build -G "Visual Studio 16 2019" -A x64
cmake --build build --config Release
build\Release\baseball_arcade.exe
```

The first CMake configure downloads raylib 5.5 through FetchContent.

## Controls

- **Space / Z / Enter** - swing
- **R** - reset the game
- **Esc** - close the window

## Notes

The reference screenshot is low-resolution/pixel-art, so the implementation intentionally uses simple block geometry and a limited palette rather than anti-aliased modern artwork. The HUD text is also positioned to reproduce the clipped side-panel look.
