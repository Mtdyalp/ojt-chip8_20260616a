#include "../include/display.h"
#include <stdio.h>
#include <string.h>

static void draw_text(DisplayState *state,
                      const char *text,
                      int x,
                      int y)
{
    SDL_Color color = {255, 255, 255, 255};

    SDL_Surface *surface =
        TTF_RenderUTF8_Blended(state->font, text, color);

    if (!surface) {
        printf("文字生成失败: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture *texture =
        SDL_CreateTextureFromSurface(state->renderer, surface);

    if (!texture) {
        printf("文字纹理创建失败: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dst = {
        x,
        y,
        surface->w,
        surface->h
    };

    SDL_RenderCopy(state->renderer, texture, NULL, &dst);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

static void draw_debug_panel(DisplayState *state,
                             chip8_t *cpu,
                             const char *debug_status)
{
    SDL_Rect panel = {
        GAME_W,
        0,
        DEBUG_PANEL_W,
        GAME_H
    };

    SDL_SetRenderDrawColor(state->renderer, 30, 30, 30, 255);
    SDL_RenderFillRect(state->renderer, &panel);

    SDL_SetRenderDrawColor(state->renderer, 100, 100, 100, 255);
    SDL_RenderDrawLine(state->renderer, GAME_W, 0, GAME_W, GAME_H);

    int x = GAME_W + 20;
    int y = 15;
    char line[128];

    if (debug_status &&
        strcmp(debug_status, "PAUSED") == 0) {

               snprintf(line, sizeof(line),
                        "[PAUSED] Step: N Resume: Space");

    } else if (debug_status &&
               strcmp(debug_status, "RUNNING") == 0) {

        snprintf(line, sizeof(line),
                 "[RUNNING] Pause: Space");

    } else {
        snprintf(line, sizeof(line),
                 "[%s]",
                 debug_status ? debug_status : "DEBUG");
   }

   draw_text(state, line, x, y);

    y += 35;
    draw_text(state, "== Registers ==", x, y);

    y += 30;

    for (int row = 0; row < 4; row++) {
        int first = row * 4;

        snprintf(line, sizeof(line),
                 "V%X:%02X  V%X:%02X  V%X:%02X  V%X:%02X",
                 first,     cpu->V[first],
                 first + 1, cpu->V[first + 1],
                 first + 2, cpu->V[first + 2],
                 first + 3, cpu->V[first + 3]);

        draw_text(state, line, x, y);
        y += 25;
    }

    y += 10;

    snprintf(line, sizeof(line),
             "I:%04X  PC:%04X  SP:%02X",
             cpu->I, cpu->PC, cpu->SP);
    draw_text(state, line, x, y);

    y += 30;

    snprintf(line, sizeof(line),
             "DT:%02X  ST:%02X",
             cpu->delay_timer,
             cpu->sound_timer);
    draw_text(state, line, x, y);
}

int display_init(DisplayState *state, int debug_mode)
{
    state->window = NULL;
    state->renderer = NULL;
    state->font = NULL;
    state->debug_mode = debug_mode;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init 失败: %s\n", SDL_GetError());
        return -1;
    }

    int window_width = debug_mode
                     ? DEBUG_WINDOW_W
                     : NORMAL_WINDOW_W;

    state->window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        WINDOW_H,
        SDL_WINDOW_SHOWN);

    if (!state->window) {
        printf("SDL_CreateWindow 失败: %s\n", SDL_GetError());
        display_cleanup(state);
        return -1;
    }

    state->renderer = SDL_CreateRenderer(
        state->window,
        -1,
        SDL_RENDERER_SOFTWARE);

    if (!state->renderer) {
        printf("SDL_CreateRenderer 失败: %s\n", SDL_GetError());
        display_cleanup(state);
        return -1;
    }

    if (debug_mode) {
        if (TTF_Init() < 0) {
            printf("TTF_Init 失败: %s\n", TTF_GetError());
            display_cleanup(state);
            return -1;
        }

        state->font = TTF_OpenFont("assets/font.ttf", 16);

        if (!state->font) {
            printf("字体加载失败: %s\n", TTF_GetError());
            display_cleanup(state);
            return -1;
        }
    }

    return 0;
}

void display_render(DisplayState *state, chip8_t *cpu, const char *debug_status)
{
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);//黑色
    SDL_RenderClear(state->renderer);//清屏

    for (int y = 0; y < CHIP8_SCREEN_H; y++) {
        for (int x = 0; x < CHIP8_SCREEN_W; x++) {
            if (cpu->screen[y * CHIP8_SCREEN_W + x]) {
                SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);//白色
                SDL_Rect rect = {x * SCALE, y * SCALE, SCALE, SCALE};//每个 CHIP-8 像素放大 SCALE 倍
                SDL_RenderFillRect(state->renderer, &rect);//画白色方块
            }
        }
    }

    if (state->debug_mode && state->font) {
        draw_debug_panel(state, cpu, debug_status);
    }

    SDL_RenderPresent(state->renderer);
}

void display_cleanup(DisplayState *state)
{
    if (state->font) {
        TTF_CloseFont(state->font);
        state->font = NULL;
    }

    if (TTF_WasInit()) {
        TTF_Quit();
    }

    if (state->renderer) {
        SDL_DestroyRenderer(state->renderer);
        state->renderer = NULL;
    }

    if (state->window) {
        SDL_DestroyWindow(state->window);
        state->window = NULL;
    }

    SDL_Quit();
}