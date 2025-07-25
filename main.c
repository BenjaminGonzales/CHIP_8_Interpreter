//
// Created by Benji on 7/21/2025.
//
#include <stdio.h>
#include <SDL2/SDL.h>
#include "include/chip8_CPU.h"

int main(int argc, char* argv[])
{
    struct chip8_cpu *emulator_handle = NULL;
    emulator_handle = vInit_CHIP8();

    for (int i = 0; i < 80; i++)
    {
        uint8_t test_int = test_get_byte(emulator_handle, i);
        printf("%0X, ", test_int);
        if ((i + 1) % 5 == 0) printf("\n");
    }
    return 0;
}