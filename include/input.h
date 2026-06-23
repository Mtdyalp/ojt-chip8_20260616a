#ifndef INPUT_H
#define INPUT_H

#include "chip8.h"
#include <SDL2/SDL.h>

/* CHIP-8 键值:        PC 键盘映射 */
/* 1  2  3  C  ->      1  2  3  4 */
/* 4  5  6  D  ->      Q  W  E  R */
/* 7  8  9  E  ->      A  S  D  F */
/* A  0  B  F  ->      Z  X  C  V */

/* 将 SDL 键码转换为 CHIP-8 键值 (0-15)，无效返回 -1 */
int input_sdl_to_chip8(SDL_KeyCode keycode);

/* 处理键盘事件 (按下/松开) */
void input_handle_event(SDL_Event *event, chip8_t *cpu);

#endif