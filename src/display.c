#include "../include/display.h"
#include <stdio.h>

int display_init(DisplayState *state)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init 失败: %s\n", SDL_GetError());
        return -1;
    }

    state->window = SDL_CreateWindow("CHIP-8",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);

    if (!state->window) {
        printf("SDL_CreateWindow 失败: %s\n", SDL_GetError());
        return -1;
    }

    state->renderer = SDL_CreateRenderer(state->window, -1, SDL_RENDERER_SOFTWARE);

    if (!state->renderer) {
        printf("SDL_CreateRenderer 失败: %s\n", SDL_GetError());
        SDL_DestroyWindow(state->window);
        return -1;
    }

    return 0;
}

void display_render(DisplayState *state, chip8_t *cpu)
{
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    SDL_RenderClear(state->renderer);

    for (int y = 0; y < CHIP8_SCREEN_H; y++) {
        for (int x = 0; x < CHIP8_SCREEN_W; x++) {
            if (cpu->screen[y * CHIP8_SCREEN_W + x]) {
                SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
                SDL_Rect rect = {x * SCALE, y * SCALE, SCALE, SCALE};
                SDL_RenderFillRect(state->renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(state->renderer);
}

void display_cleanup(DisplayState *state)
{
    if (state->renderer) SDL_DestroyRenderer(state->renderer);
    if (state->window) SDL_DestroyWindow(state->window);
    SDL_Quit();
}