#include "../include/chip8.h"
#include "../include/opcodes.h"
#include "../include/display.h"
#include <stdio.h>
#include <stdlib.h>

/* ============================================================
   显示测试：画一个点
   ============================================================ */
static void run_test_pattern(DisplayState *display)
{
    chip8_t cpu;
    chip8_init(&cpu);

    /* 在屏幕中间画一个像素 */
    cpu.screen[15 * CHIP8_SCREEN_W + 31] = 1;
    cpu.dirty = 1;

    display_render(display, &cpu);

    printf("\n===== 显示测试 =====\n");
    printf("屏幕中间有一个白色方块\n");
    printf("按 ESC 或关闭窗口退出...\n");

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) running = 0;
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
        run_test_pattern(&display);
        display_cleanup(&display);
        printf("测试结束\n");
#endif        
        return 0;
    }

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
    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) running = 0;
            }
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