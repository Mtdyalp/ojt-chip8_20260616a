#include "../include/chip8.h"
#include "../include/opcodes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>  

/* ============================================================
           系统类 (0x0000)
   ============================================================ */
// 00E0 — 清屏
void op_00e0(chip8_t *cpu, uint16_t op)
{
    (void)op;
//    printf("[CLS] 清屏\n");   /* 加这行 */
    memset(cpu->screen, 0, sizeof(cpu->screen));
    cpu->dirty = 1;// 屏幕变了！
}

// 00EE — 从子程序返回
void op_00ee(chip8_t *cpu, uint16_t op)
{
    (void)op;
    if (cpu->SP > 0) {
        cpu->SP--;
        cpu->PC = cpu->stack[cpu->SP];
    }
}

/* ============================================================
           控制流类
   ============================================================ */
// 1NNN — 跳转到 nnn
void op_1nnn(chip8_t *cpu, uint16_t op)
{
    cpu->PC = NNN(op);
}

// 2NNN — 调用子程序 nnn
void op_2nnn(chip8_t *cpu, uint16_t op)
{
    if (cpu->SP < 16) {
        cpu->stack[cpu->SP] = cpu->PC;
        cpu->SP++;
        cpu->PC = NNN(op);
    }
}

// 3XKK — 如果 Vx == kk, 跳过下一条指令
void op_3xkk(chip8_t *cpu, uint16_t op)
{
    if (cpu->V[X(op)] == KK(op))
        cpu->PC += 2;
}

// 4XKK — 如果 Vx != kk, 跳过下一条指令
void op_4xkk(chip8_t *cpu, uint16_t op)
{
    if (cpu->V[X(op)] != KK(op))
        cpu->PC += 2;
}

// 5XY0 — 如果 Vx == Vy, 跳过下一条指令
void op_5xy0(chip8_t *cpu, uint16_t op)
{
    if (cpu->V[X(op)] == cpu->V[Y(op)])
        cpu->PC += 2;
}

// 9XY0 — 如果 Vx != Vy, 跳过下一条指令
void op_9xy0(chip8_t *cpu, uint16_t op)
{
    if (cpu->V[X(op)] != cpu->V[Y(op)])
        cpu->PC += 2;
}

// BNNN — 跳转到 nnn + V0
void op_bnnn(chip8_t *cpu, uint16_t op)
{
    cpu->PC = NNN(op) + cpu->V[0];
}

/* ============================================================
   寄存器操作类
   ============================================================ */
// 6XKK — Vx = kk
void op_6xkk(chip8_t *cpu, uint16_t op)
{
    cpu->V[X(op)] = KK(op);
}

// 7XKK — Vx += kk  
void op_7xkk(chip8_t *cpu, uint16_t op)
{
    cpu->V[X(op)] += KK(op);
}

// 8XY0 — Vx = Vy
void op_8xy0(chip8_t *cpu, uint16_t op)
{
    cpu->V[X(op)] = cpu->V[Y(op)];
}

// 8XY1 — Vx |= Vy
void op_8xy1(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op);
    cpu->V[x] |= cpu->V[y];
    cpu->V[0xF] = 0;
}

// 8XY2 — Vx &= Vy
void op_8xy2(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op);
    cpu->V[x] &= cpu->V[y];
    cpu->V[0xF] = 0;
}

// 8XY3 — Vx ^= Vy
void op_8xy3(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op);
    cpu->V[x] ^= cpu->V[y];
    cpu->V[0xF] = 0;
}

// 8XY4 — Vx += Vy, VF = carry
void op_8xy4(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op);
    uint16_t sum = cpu->V[x] + cpu->V[y];
    cpu->V[0xF] = (sum > 0xFF) ? 1 : 0;
    cpu->V[x] = sum & 0xFF;
}

// 8XY5 — Vx -= Vy, VF = NOT borrow
void op_8xy5(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op);
    cpu->V[0xF] = (cpu->V[x] >= cpu->V[y]) ? 1 : 0;
    cpu->V[x] -= cpu->V[y];
}

// 8XY6 — Vx >>= 1, VF = LSB
void op_8xy6(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op);
    cpu->V[0xF] = cpu->V[x] & 0x01;
    cpu->V[x] >>= 1;
}

// 8XY7 — Vx = Vy - Vx, VF = NOT borrow
void op_8xy7(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op);
    cpu->V[0xF] = (cpu->V[y] >= cpu->V[x]) ? 1 : 0;
    cpu->V[x] = cpu->V[y] - cpu->V[x];
}

// 8XYE — Vx <<= 1, VF = MSB
void op_8xye(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op);
    cpu->V[0xF] = (cpu->V[x] & 0x80) ? 1 : 0;
    cpu->V[x] <<= 1;
}

/* ============================================================
    内存操作类
   ============================================================ */
// ANNN — I = nnn
void op_annn(chip8_t *cpu, uint16_t op)
{
    cpu->I = NNN(op);
}

/* ============================================================
   随机数
   ============================================================ */
// CXKK — Vx = random byte AND kk
void op_cxkk(chip8_t *cpu, uint16_t op)
{
    cpu->V[X(op)] = (rand() & 0xFF) & KK(op);
}

/* ============================================================
    描画类
   ============================================================ */
// DXYN — 绘制精灵
void op_dxyn(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op), n = N(op);
    uint8_t sx = cpu->V[x] % 64;
    uint8_t sy = cpu->V[y] % 32;

//    printf("[DRAW] X=%d Y=%d N=%d I=0x%04X\n", sx, sy, n, cpu->I);

    /* 打印精灵数据（最多 16 行） */
 //   printf("[DRAW] 精灵数据: ");
 #ifdef DEBUG
    int max_rows = (n < 16) ? n : 16;
    for (int i = 0; i < max_rows; i++) {
        printf("%02X ", cpu->mem[cpu->I + i]);
    }
    printf("\n");
#endif

    cpu->V[0xF] = 0;

    int pixel_count = 0;  /* 统计画了多少像素 */
    for (int row = 0; row < n; row++) {
        uint8_t byte = cpu->mem[cpu->I + row];/* 取出精灵数据的一行 */
        /* 1字节8位 就是8个像素 */
        for (int col = 0; col < 8; col++) {
            if (byte & (0x80 >> col)) {
                int px = (sx + col) % 64;
                int py = (sy + row) % 32;
                int idx = py * 64 + px;//跳过前面 py 行,在当前行向右移 px 列

                if (cpu->screen[idx])// 屏幕原来这里已经亮
                    cpu->V[0xF] = 1;// 发生重叠/碰撞

                cpu->screen[idx] ^= 1;// 反转当前像素
                pixel_count++;
            }
        }
    }

//    printf("[DRAW] 本次绘制了 %d 个像素\n", pixel_count);
    cpu->dirty = 1;
}

/* ============================================================
    键盘类
   ============================================================ */

// EX9E — 如果按下就跳一条指令
void op_ex9e(chip8_t *cpu, uint16_t op)
{
    if (cpu->key_down[cpu->V[X(op)]])
        cpu->PC += 2;
}

// EXA1 — 如果没按就跳一条指令
void op_exa1(chip8_t *cpu, uint16_t op)
{
    if (!cpu->key_down[cpu->V[X(op)]])
        cpu->PC += 2;
}

// FX0A — 等待按键，按下后存入 Vx
void op_fx0a(chip8_t *cpu, uint16_t op)
{
    cpu->key_wait_value = X(op);
    cpu->key_waiting = 1;
}
/* ============================================================
    定时器类
   ============================================================ */
// FX07 — 将延迟定时器的值存入 Vx
void op_fx07(chip8_t *cpu, uint16_t op)
{
    cpu->V[X(op)] = cpu->delay_timer;
}

// FX15 — 将 Vx 的值存入延迟定时器
void op_fx15(chip8_t *cpu, uint16_t op)
{
    cpu->delay_timer = cpu->V[X(op)];
}

// FX18 — 将 Vx 的值存入声音定时器
void op_fx18(chip8_t *cpu, uint16_t op)
{
    cpu->sound_timer = cpu->V[X(op)];
}


// FX1E — 将 Vx 的值加到 I 上
void op_fx1e(chip8_t *cpu, uint16_t op)
{
    cpu->I += cpu->V[X(op)];
}

// FX29 — 将 I 设置为 Vx 字体的地址
void op_fx29(chip8_t *cpu, uint16_t op)
{
    cpu->I = cpu->V[X(op)] * 5;
}

// FX33 — 将 Vx 的值转换为 BCD 存入 I, I+1, I+2
void op_fx33(chip8_t *cpu, uint16_t op)
{
    uint8_t val = cpu->V[X(op)];
    cpu->mem[cpu->I] = val / 100;
    cpu->mem[cpu->I + 1] = (val / 10) % 10;
    cpu->mem[cpu->I + 2] = val % 10;
}

// FX55 — 将 V0~Vx 的值存入 I 开始的内存
void op_fx55(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op);
    for (int i = 0; i <= x; i++)
        cpu->mem[cpu->I + i] = cpu->V[i];
}

// FX65 — 将 I 开始的内存存入 V0~Vx
void op_fx65(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op);
    for (int i = 0; i <= x; i++)
        cpu->V[i] = cpu->mem[cpu->I + i];
}

