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
    uint32_t pixels[SCREEN_WIDTH][SCREEN_HEIGHT]; // apparently 32 bit ints work best with SDL(?)
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

// returns a display struct w/ window & renderer. Returns NULL on failure.
display_t *p_display_init(void)
{
    display_t *p_display_ret = NULL;

    p_display_ret = calloc(sizeof(display_t), 1);
    if (p_display_ret == NULL)
    {
        perror("malloc failed on display struct allocation\n");
        return NULL;
    }
    p_display_ret->window = NULL;
    p_display_ret->renderer = NULL;

    iSdl_init(p_display_ret);
    return p_display_ret;
}

int clear_screen(const display_t *display)
{
    SDL_RenderClear(display->renderer);
    return 0;
}
//
int draw_internal(display_t *display, int x, int y)
{
    const uint32_t pixel_start_on = display->pixels[x][y];
    display->pixels[x][y] ^= 0xFFFFFFFF;
    const uint32_t pixel_end_on = display->pixels[x][y];

    if (pixel_start_on && !pixel_end_on)
    {
        SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
        SDL_RenderDrawPoint(display->renderer, x, y);
        return 1;
    }
    if (!pixel_start_on && pixel_end_on)
    {
        SDL_SetRenderDrawColor(display->renderer, 255, 255, 255, 255);
        SDL_RenderDrawPoint(display->renderer, x, y);
    }
    return 0;
}

int draw(const display_t *display)
{
    SDL_RenderPresent(display->renderer);
    return 0;
}

int make_sound(const display_t *display)
{
    return 1;
}