//
// Created by Benji on 7/21/2025.
//
#include <stdio.h>
#include <SDL2/SDL.h>
#include "include/chip8_CPU.h"
#include "include/display.h"
#include "include/logging.h"

int main(int argc, char* argv[])
{
    struct chip8_cpu *emulator = p_Init_CHIP8();
    display_t *p_display = p_display_init();
    set_display(emulator, p_display);

    // set_log_level(DEBUG);

    FILE *infile = fopen(argv[1], "rb");
    const struct gamefile *game = p_load_game_from_file(infile);
    if (game == NULL)
    {
        perror("game read error!");
        return -1;
    }
    v_load_rom(emulator, game);

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
                    v_handle_keyboard_interrupt(emulator, &event.key);
                    break;
                case SDL_KEYUP:
                    v_handle_keyboard_interrupt(emulator, &event.key);
                default:
                    break;
            }
        }

        const uint16_t instruction = i_fetch_instruction(emulator);
        if (!i_emulator_is_waiting(emulator))
        {
            decode(emulator, instruction);
        }
        draw_thru_emulator(emulator);
        SDL_Delay(32);
    }

    return 0;
}

