//
// Created by Benji on 7/21/2025.
//
#include <stdio.h>
#include <SDL2/SDL.h>
#include "include/chip8_CPU.h"
#include "include/display.h"

int f_clear_screen = 0;

int main(int argc, char* argv[])
{
    struct chip8_cpu *chip8_cpu = p_Init_CHIP8();
    display_t *p_display = p_display_init();

    // main loop, for now!
    while (1)
    {
        uint16_t instruction = i_fetch_instruction(chip8_cpu);
        decode(instruction);
    }

    return 0;
}

