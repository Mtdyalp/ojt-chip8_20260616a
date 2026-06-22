#ifndef DISPLAY_H
#define DISPLAY_H

#include "chip8.h"
#include <SDL2/SDL.h>

#define SCALE 10
#define WINDOW_W (64 * SCALE)
#define WINDOW_H (32 * SCALE)

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} DisplayState;

int  display_init(DisplayState *state);
void display_render(DisplayState *state, chip8_t *cpu);
void display_cleanup(DisplayState *state);

#endif