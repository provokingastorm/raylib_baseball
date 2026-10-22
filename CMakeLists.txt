cmake_minimum_required(VERSION 3.16)
project(BaseballArcadeRaylib C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_executable(baseball_arcade
    src/main.c
)

target_link_libraries(baseball_arcade PRIVATE raylib)

if (MSVC)
    target_compile_options(baseball_arcade PRIVATE /W4 /wd4996)
else()
    target_compile_options(baseball_arcade PRIVATE -Wall -Wextra -Wpedantic)
endif()
