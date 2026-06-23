#include "../include/chip8.h"
#include "../include/opcodes.h"
#include "../include/display.h"
#include "../include/input.h"
#include <stdio.h>
#include <stdlib.h>

/* ============================================================
   显示测试：画一个点
   ============================================================ */
static void run_test_pattern(DisplayState *display, chip8_t *cpu)
{
    cpu->screen[15 * CHIP8_SCREEN_W + 31] = 1;
    cpu->dirty = 1;
    display_render(display, cpu);

    printf("\n===== 显示测试 =====\n");
    printf("屏幕中间有一个白色方块\n");
    printf("按映射键查看 CHIP-8 键值，ESC 退出\n");

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                // ESC
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                } else {
                    input_handle_event(&event, cpu);
                    int key = input_sdl_to_chip8(event.key.keysym.sym);
                    if (key >= 0) {
                        printf("[KEY] %-8s → CHIP-8 0x%X\n",
                               SDL_GetKeyName(event.key.keysym.sym), key);
                    }
                }
            } else {
                input_handle_event(&event, cpu);
            }
        }
        SDL_Delay(16);
    }
}

int main(int argc, char *argv[])
{
    /* ===== CPU 初始化提到前面，两种模式共用 ===== */
    chip8_t cpu;
    chip8_init(&cpu);

    DisplayState display;
    if (display_init(&display) != 0) {
        return 1;
    }

    if (argc < 2) {
#if 1        
        printf("===== 显示测试模式 =====\n");
        printf("提示: 运行 ROM 请使用 ./chip8 <rom文件>\n");
        run_test_pattern(&display, &cpu);
        display_cleanup(&display);
        printf("测试结束\n");
#endif        
        return 0;
    }

    /* ===== 正常模式 ===== */
    if (!chip8_load(&cpu, argv[1])) {
        printf("加载 ROM 失败\n");
        display_cleanup(&display);
        return 1;
    }

    printf("===== CHIP-8 模拟器 =====\n");
    printf("ROM: %s\n", argv[1]);
    printf("PC = 0x%04X\n", cpu.PC);
    printf("按 ESC 或关闭窗口退出\n");
#if 0
    //printf("键盘映射: Q=左, E=右, 1/2/3/4=按键\n");
    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) running = 0;
            }

            /* 键盘输入交给 input.c 处理 */
            //input_handle_event(&event, &cpu);
        }

        for (int i = 0; i < 10; i++) {
            chip8_emulate_cycle(&cpu);
        }

        display_render(&display, &cpu);
        cpu.dirty = 0;
        SDL_Delay(16);
    }
#endif
    display_cleanup(&display);
    printf("模拟器已退出\n");
    return 0;
}