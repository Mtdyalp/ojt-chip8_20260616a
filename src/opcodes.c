#include "../include/chip8.h"
#include "../include/opcodes.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================
           系统类 (0x0000)
   ============================================================ */

void op_00e0(chip8_t *cpu, uint16_t op)
{
    (void)op;
    memset(cpu->screen, 0, sizeof(cpu->screen));
    cpu->dirty = 1;
}

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

void op_1nnn(chip8_t *cpu, uint16_t op)
{
    cpu->PC = NNN(op);
}

void op_2nnn(chip8_t *cpu, uint16_t op)
{
    if (cpu->SP < 16) {
        cpu->stack[cpu->SP] = cpu->PC;
        cpu->SP++;
        cpu->PC = NNN(op);
    }
}

void op_3xkk(chip8_t *cpu, uint16_t op)
{
    if (cpu->V[X(op)] == KK(op))
        cpu->PC += 2;
}

void op_4xkk(chip8_t *cpu, uint16_t op)
{
    if (cpu->V[X(op)] != KK(op))
        cpu->PC += 2;
}

void op_5xy0(chip8_t *cpu, uint16_t op)
{
    if (cpu->V[X(op)] == cpu->V[Y(op)])
        cpu->PC += 2;
}

void op_9xy0(chip8_t *cpu, uint16_t op)
{
    if (cpu->V[X(op)] != cpu->V[Y(op)])
        cpu->PC += 2;
}

void op_bnnn(chip8_t *cpu, uint16_t op)
{
    cpu->PC = NNN(op) + cpu->V[0];
}

/* ============================================================
   寄存器操作类
   ============================================================ */

void op_6xkk(chip8_t *cpu, uint16_t op)
{
    cpu->V[X(op)] = KK(op);
}

void op_7xkk(chip8_t *cpu, uint16_t op)
{
    cpu->V[X(op)] += KK(op);
}

/* ============================================================
    算术逻辑类 (0x8000)
   ============================================================ */

void op_8xy0(chip8_t *cpu, uint16_t op)
{
    cpu->V[X(op)] = cpu->V[Y(op)];
}

void op_8xy1(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op);
    cpu->V[x] |= cpu->V[y];
    cpu->V[0xF] = 0;
}

void op_8xy2(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op);
    cpu->V[x] &= cpu->V[y];
    cpu->V[0xF] = 0;
}

void op_8xy3(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op);
    cpu->V[x] ^= cpu->V[y];
    cpu->V[0xF] = 0;
}

void op_8xy4(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op);
    uint16_t sum = cpu->V[x] + cpu->V[y];
    cpu->V[0xF] = (sum > 0xFF) ? 1 : 0;
    cpu->V[x] = sum & 0xFF;
}

void op_8xy5(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op);
    cpu->V[0xF] = (cpu->V[x] >= cpu->V[y]) ? 1 : 0;
    cpu->V[x] -= cpu->V[y];
}

void op_8xy6(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op);
    cpu->V[0xF] = cpu->V[x] & 0x01;
    cpu->V[x] >>= 1;
}

void op_8xy7(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op);
    cpu->V[0xF] = (cpu->V[y] >= cpu->V[x]) ? 1 : 0;
    cpu->V[x] = cpu->V[y] - cpu->V[x];
}

void op_8xye(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op);
    cpu->V[0xF] = (cpu->V[x] & 0x80) ? 1 : 0;
    cpu->V[x] <<= 1;
}

/* ============================================================
    内存操作类
   ============================================================ */

void op_annn(chip8_t *cpu, uint16_t op)
{
    cpu->I = NNN(op);
}

/* ============================================================
   随机数
   ============================================================ */

void op_cxkk(chip8_t *cpu, uint16_t op)
{
    cpu->V[X(op)] = (rand() & 0xFF) & KK(op);
}

/* ============================================================
    描画类
   ============================================================ */

void op_dxyn(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op), y = Y(op), n = N(op);
    uint8_t sx = cpu->V[x] % 64;
    uint8_t sy = cpu->V[y] % 32;

    cpu->V[0xF] = 0;

    for (int row = 0; row < n; row++) {
        uint8_t byte = cpu->mem[cpu->I + row];
        for (int col = 0; col < 8; col++) {
            if (byte & (0x80 >> col)) {
                int px = (sx + col) % 64;
                int py = (sy + row) % 32;
                int idx = py * 64 + px;

                if (cpu->screen[idx])
                    cpu->V[0xF] = 1;

                cpu->screen[idx] ^= 1;
            }
        }
    }

    cpu->dirty = 1;
}

/* ============================================================
    键盘类
   ============================================================ */

void op_ex9e(chip8_t *cpu, uint16_t op)
{
    if (cpu->key_down[cpu->V[X(op)]])
        cpu->PC += 2;
}

void op_exa1(chip8_t *cpu, uint16_t op)
{
    if (!cpu->key_down[cpu->V[X(op)]])
        cpu->PC += 2;
}

/* ============================================================
   杂项类 (0xF000)
   ============================================================ */

void op_fx07(chip8_t *cpu, uint16_t op)
{
    cpu->V[X(op)] = cpu->delay_timer;
}

void op_fx0a(chip8_t *cpu, uint16_t op)
{
    cpu->key_wait_value = X(op);
    cpu->key_waiting = 1;
}

void op_fx15(chip8_t *cpu, uint16_t op)
{
    cpu->delay_timer = cpu->V[X(op)];
}

void op_fx18(chip8_t *cpu, uint16_t op)
{
    cpu->sound_timer = cpu->V[X(op)];
}

void op_fx1e(chip8_t *cpu, uint16_t op)
{
    cpu->I += cpu->V[X(op)];
}

void op_fx29(chip8_t *cpu, uint16_t op)
{
    cpu->I = cpu->V[X(op)] * 5;
}

void op_fx33(chip8_t *cpu, uint16_t op)
{
    uint8_t val = cpu->V[X(op)];
    cpu->mem[cpu->I] = val / 100;
    cpu->mem[cpu->I + 1] = (val / 10) % 10;
    cpu->mem[cpu->I + 2] = val % 10;
}

void op_fx55(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op);
    for (int i = 0; i <= x; i++)
        cpu->mem[cpu->I + i] = cpu->V[i];
}

void op_fx65(chip8_t *cpu, uint16_t op)
{
    uint8_t x = X(op);
    for (int i = 0; i <= x; i++)
        cpu->V[i] = cpu->mem[cpu->I + i];
}

