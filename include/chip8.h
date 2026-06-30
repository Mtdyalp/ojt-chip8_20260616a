#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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
    uint16_t I;// 数据的位置
    uint16_t PC;// 指令的位置

    /* 调用栈 */
    uint16_t stack[CHIP8_STACK_DEPTH];
    uint8_t SP;// 栈指针

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
    uint8_t key_wait_value;        // 等待按键时，按下的键存入 Vx

    /* 模拟器状态 */
    int running;
} chip8_t;

/* ============================================================
   接口
   ============================================================ */

void chip8_init(chip8_t *cpu);
int  chip8_load(chip8_t *cpu, const char *path);
void chip8_emulate_cycle(chip8_t *cpu);
void chip8_execute_opcode(chip8_t *cpu, uint16_t opcode);
void chip8_disassemble(uint16_t op, char *text, size_t text_size);
#endif