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
    set_display(chip8_cpu, p_display);

    // main loop, for now!
    int loop = 1;
    while (loop)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    loop = 0;
                    break;
                case SDL_KEYDOWN:
                    v_handle_keyboard_interrupt(chip8_cpu, &event.key);
                    break;
                case SDL_KEYUP:
                    v_handle_keyboard_interrupt(chip8_cpu, &event.key);
                default:
                    break;
            }
        }

        const uint16_t instruction = i_fetch_instruction(chip8_cpu);
        if (!i_emulator_is_waiting(chip8_cpu))
        {
            decode(chip8_cpu, instruction);
        }
        SDL_Delay(64);
    }

    return 0;
}

