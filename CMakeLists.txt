cmake_minimum_required(VERSION 3.30)
project(CHIP_8_Interpreter C)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_C_STANDARD 11)

find_package(SDL2 CONFIG REQUIRED)

add_executable(CHIP_8_Interpreter
    main.c)
