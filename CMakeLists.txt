cmake_minimum_required(VERSION 3.30)
project(CHIP_8_Interpreter C)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_C_STANDARD 11)

find_package(SDL2 CONFIG REQUIRED)

add_library(chip8_CPU src/chip8_CPU.c)

add_executable(CHIP_8_Interpreter
    main.c
        src/chip8_CPU.c
        include/chip8_CPU.h)

add_executable(TEST_GAME
        src/sdl_test.c)

target_link_libraries(CHIP_8_Interpreter PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(TEST_GAME PRIVATE SDL2::SDL2 SDL2::SDL2main)
