//
// Created by Benji on 7/24/2025.
//

#include "../include/display.h"
#include <SDL2/SDL.h>
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALING_FACTOR 10
#define SDL_HINT_RENDER_SCALE_QUALITY 1

struct Display {
    SDL_Window *window;
    SDL_Renderer *renderer;
    uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT] = {0}; // apparently 32 bit ints work best with SDL(?)
};

void vSdl_shutdown(display_t const *display)
{
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
    SDL_Quit();
}

int iSdl_init(display_t *p_display)
{
    int init_status = SDL_Init(SDL_INIT_EVERYTHING); // status 0 = success, < 0 = error
    if (init_status < 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    p_display->window = SDL_CreateWindow(
        "test_title_window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * SCALING_FACTOR,
        SCREEN_HEIGHT * SCALING_FACTOR,
        0);
    if (p_display->window == NULL)
    {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return -1;
    }

    p_display->renderer = SDL_CreateRenderer(
        p_display->window,
        -1,
        0);
    if (p_display->renderer == NULL)
    {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return -1;
    }
    SDL_RenderSetScale(p_display->renderer, SCALING_FACTOR, SCALING_FACTOR);

    return 0;
}




int main(int argc, char *argv[])
{
    display_t emulator = {
    .window = NULL,
    .renderer = NULL
    };

    int init_status = iSdl_init(&emulator);
    if (init_status != 0)
    {
        vSdl_shutdown(&emulator);
        printf("initialization failure!\n");
        exit(EXIT_FAILURE);
    }

    int x = 0, y = 0, r = 0, g = 0, b = 0;

    enum shift_color
    {
        RED,
        GREEN,
        BLUE,
    };
    enum shift_color color = RED;

    // main loop
    int loop = 1;

    while (loop)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                loop = 0;
                break;
            }
        }
        SDL_SetRenderDrawColor(emulator.renderer, 255, 255, 255, 255);
        SDL_RenderClear(emulator.renderer);
        SDL_SetRenderDrawColor(emulator.renderer, r, g, b, 255);
        SDL_RenderDrawPoint(emulator.renderer, x, y);
        SDL_RenderPresent(emulator.renderer);

        x += 1;
        if (x >= SCREEN_WIDTH)
        {
            x = 0;
            y = (y + 1) % SCREEN_HEIGHT;
        }
        SDL_Delay(32);
    }
    vSdl_shutdown(&emulator);
    exit(EXIT_SUCCESS);
}
