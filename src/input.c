#include "../include/input.h"
#include <stdio.h>

/* CHIP-8 键 -> PC 键映射表 (顺序: 0x0 ~ 0xF) */
static const SDL_KeyCode keymap[16] = {
    SDLK_x,      /* 0x0 */
    SDLK_1,      /* 0x1 */
    SDLK_2,      /* 0x2 */
    SDLK_3,      /* 0x3 */
    SDLK_q,      /* 0x4 */
    SDLK_w,      /* 0x5 */
    SDLK_e,      /* 0x6 */
    SDLK_a,      /* 0x7 */
    SDLK_s,      /* 0x8 */
    SDLK_d,      /* 0x9 */
    SDLK_z,      /* 0xA */
    SDLK_c,      /* 0xB */
    SDLK_4,      /* 0xC */
    SDLK_r,      /* 0xD */
    SDLK_f,      /* 0xE */
    SDLK_v       /* 0xF */
};

//如：把 SDL 底层扫描的 SDLK_q 这种物理键码，转成 CHIP-8 认识的 0x4
int input_sdl_to_chip8(SDL_KeyCode keycode)
{
    for (int i = 0; i < 16; i++) {
        if (keymap[i] == keycode) {
            return i;
        }
    }
    return -1;
}

void input_handle_event(SDL_Event *event, chip8_t *cpu)
{
    if (event->type == SDL_KEYDOWN) {
        int key = input_sdl_to_chip8(event->key.keysym.sym);
        if (key >= 0) {
            cpu->key_down[key] = 1;//标记按下

            /* Fx0A: 等待按键 (CPU 正等待按键) */
            if (cpu->key_waiting) {
                cpu->key_waiting = 0;
                cpu->V[cpu->key_wait_value] = key;
            }
        }
    } else if (event->type == SDL_KEYUP) {
        int key = input_sdl_to_chip8(event->key.keysym.sym);
        if (key >= 0) {
            cpu->key_down[key] = 0;//标记松开
        }
    }
}