#include "../include/chip8.h"
#include "../include/opcodes.h"
#include "../include/display.h"
#include "../include/input.h"
#include "../include/audio.h"
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

    cpu->delay_timer = 60;
    cpu->sound_timer = 60;

    printf("\n===== 显示/定时器测试 =====\n");
    printf("屏幕中间有一个白色方块\n");
    printf("DT/ST 从 60 开始递减，约 1 秒到 0\n");
    printf("sound_timer > 0 时蜂鸣，ESC 退出\n");

    int running = 1;
    SDL_Event event;
    int last_dt = -1;
    int last_st = -1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                } else {
                    int key = input_sdl_to_chip8(event.key.keysym.sym);
                    if (key >= 0) {
                        printf("[KEY] %s -> CHIP-8 0x%X\n",
                               SDL_GetKeyName(event.key.keysym.sym), key);
                        cpu->key_down[key] = 1;
                    }
                }
            } else if (event.type == SDL_KEYUP) {
                int key = input_sdl_to_chip8(event.key.keysym.sym);
                if (key >= 0) {
                    cpu->key_down[key] = 0;
                }
            }
        }

        if (cpu->delay_timer > 0) cpu->delay_timer--;
        if (cpu->sound_timer > 0) cpu->sound_timer--;

        audio_update(cpu->sound_timer);

        if (cpu->delay_timer != last_dt || cpu->sound_timer != last_st) {
            printf("[TIMER] DT=%d ST=%d\n",
                   cpu->delay_timer, cpu->sound_timer);
            last_dt = cpu->delay_timer;
            last_st = cpu->sound_timer;
        }

        SDL_Delay(16);
    }
}

int main(int argc, char *argv[])
{
    /*CPU 初始化*/
    chip8_t cpu;
    chip8_init(&cpu);

    /* 初始化显示 */
    DisplayState display;
    if (display_init(&display) != 0) {
        return 1;
    }

    /* 初始化音频 */
    audio_init();

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

#if 1
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
    printf("键盘映射: Q=左, E=右, 1/2/3/4=按键\n");

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) running = 0;//ESC
            }

            /* 键盘输入交给 input.c 处理 */
            input_handle_event(&event, &cpu);
        }

        for (int i = 0; i < 10; i++) {
            chip8_emulate_cycle(&cpu);//每帧先执行若干条 CHIP-8 指令
        }

        if (cpu.delay_timer > 0) cpu.delay_timer--;
        if (cpu.sound_timer > 0) cpu.sound_timer--;
        /* 更新蜂鸣状态（根据 sound_timer） */
        audio_update(cpu.sound_timer);

        /* 画完了 更新窗口 */
        display_render(&display, &cpu);
        cpu.dirty = 0;

        /* 60Hz 速度控制 */
        SDL_Delay(16);
    }
#endif
    /* 清理 */
    display_cleanup(&display);
    audio_cleanup();
    printf("模拟器已退出\n");
    return 0;
}