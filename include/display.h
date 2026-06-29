#ifndef DISPLAY_H
#define DISPLAY_H

#include "chip8.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCALE 10

#define GAME_W  (CHIP8_SCREEN_W * SCALE)
#define GAME_H  (CHIP8_SCREEN_H * SCALE)
#define DEBUG_PANEL_W 360

#define NORMAL_WINDOW_W GAME_W
#define DEBUG_WINDOW_W  (GAME_W + DEBUG_PANEL_W)
#define WINDOW_H GAME_H

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    int debug_mode;
} DisplayState;

int display_init(DisplayState *state, int debug_mode);

void display_render(DisplayState *state,
                    chip8_t *cpu,
                    const char *debug_status);

void display_cleanup(DisplayState *state);

#endif