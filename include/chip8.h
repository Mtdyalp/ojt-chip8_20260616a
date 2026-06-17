#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
   常量定义
   ============================================================ */

#define CHIP8_MEM_SIZE      4096
#define CHIP8_PROG_START    0x200
#define CHIP8_SCREEN_W      64
#define CHIP8_SCREEN_H      32
#define CHIP8_PIXEL_COUNT   (CHIP8_SCREEN_W * CHIP8_SCREEN_H)
#define CHIP8_STACK_DEPTH   16
#define CHIP8_REG_COUNT     16
#define CHIP8_FONT_BYTES    80

/* ============================================================
   CPU 状态
   ============================================================ */

typedef struct {
    /* 寄存器 */
    uint8_t V[CHIP8_REG_COUNT];
    uint16_t I;
    uint16_t PC;

    /* 调用栈 */
    uint16_t stack[CHIP8_STACK_DEPTH];
    uint8_t SP;

    /* 内存 + 显示 */
    uint8_t mem[CHIP8_MEM_SIZE];
    uint8_t screen[CHIP8_PIXEL_COUNT];
    int dirty;                     /* 1 = 需要刷新画面 */

    /* 定时器 */
    uint8_t delay_timer;
    uint8_t sound_timer;

    /* 键盘 */
    int key_down[16];              /* 1=按下 */
    int key_waiting;               /* 1=正在等按键 (FX0A) */
    uint8_t key_wait_value;

    /* 模拟器状态 */
    int running;
} chip8_t;

/* ============================================================
   接口
   ============================================================ */

void chip8_init(chip8_t *cpu);
int  chip8_load_rom(chip8_t *cpu, const char *path);

#endif