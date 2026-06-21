#include "../include/chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void chip8_execute_opcode(chip8_t *cpu, uint16_t opcode)
{
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t kk = opcode & 0x00FF;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = opcode & 0x000F;

    switch (opcode & 0xF000) {
        /* ============================================================
           系统类
           ============================================================ */
        case 0x0000:
            if (opcode == 0x00E0) {
                memset(cpu->screen, 0, sizeof(cpu->screen));
                cpu->dirty = 1;
            } else if (opcode == 0x00EE) {
                if (cpu->SP > 0) {
                    cpu->SP--;
                    cpu->PC = cpu->stack[cpu->SP];
                }
            }
            break;

        /* ============================================================
           控制流类
           ============================================================ */
        case 0x1000: cpu->PC = nnn; break;
        case 0x2000:
            if (cpu->SP < 16) {
                cpu->stack[cpu->SP] = cpu->PC;
                cpu->SP++;
                cpu->PC = nnn;
            }
            break;
        case 0x3000: if (cpu->V[x] == kk) cpu->PC += 2; break;
        case 0x4000: if (cpu->V[x] != kk) cpu->PC += 2; break;
        case 0x5000: if (cpu->V[x] == cpu->V[y]) cpu->PC += 2; break;
        case 0x9000: if (cpu->V[x] != cpu->V[y]) cpu->PC += 2; break;
        case 0xB000: cpu->PC = nnn + cpu->V[0]; break;

        /* ============================================================
           寄存器操作类
           ============================================================ */
        case 0x6000: cpu->V[x] = kk; break;
        case 0x7000: cpu->V[x] += kk; break;

        case 0x8000:
            switch (n) {
                case 0x0: cpu->V[x] = cpu->V[y]; break;
                case 0x1: cpu->V[x] |= cpu->V[y]; cpu->V[0xF] = 0; break;
                case 0x2: cpu->V[x] &= cpu->V[y]; cpu->V[0xF] = 0; break;
                case 0x3: cpu->V[x] ^= cpu->V[y]; cpu->V[0xF] = 0; break;
                case 0x4: {
                    uint16_t result = cpu->V[x] + cpu->V[y];
                    cpu->V[0xF] = (result > 0xFF) ? 1 : 0;
                    cpu->V[x] = result & 0xFF;
                    break;
                }
                case 0x5:
                    cpu->V[0xF] = (cpu->V[x] >= cpu->V[y]) ? 1 : 0;
                    cpu->V[x] -= cpu->V[y];
                    break;
                case 0x6:
                    cpu->V[0xF] = cpu->V[x] & 0x01;
                    cpu->V[x] >>= 1;
                    break;
                case 0x7:
                    cpu->V[0xF] = (cpu->V[y] >= cpu->V[x]) ? 1 : 0;
                    cpu->V[x] = cpu->V[y] - cpu->V[x];
                    break;
                case 0xE:
                    cpu->V[0xF] = (cpu->V[x] & 0x80) ? 1 : 0;
                    cpu->V[x] <<= 1;
                    break;
                default: break;
            }
            break;

        /* ============================================================
           内存操作类（部分）
           ============================================================ */
        case 0xA000: cpu->I = nnn; break;

        /* ============================================================
           随机数
           ============================================================ */
        case 0xC000:  /* Cxkk: RND Vx, kk */
            cpu->V[x] = (rand() & 0xFF) & kk;
            break;

        /* ============================================================
           描画类
           ============================================================ */
        case 0xD000: {
            uint8_t x_pos = cpu->V[x] % 64;
            uint8_t y_pos = cpu->V[y] % 32;
            cpu->V[0xF] = 0;
            for (int row = 0; row < n; row++) {
                uint8_t sprite_byte = cpu->mem[cpu->I + row];
                for (int col = 0; col < 8; col++) {
                    if (sprite_byte & (0x80 >> col)) {
                        int pixel_x = (x_pos + col) % 64;
                        int pixel_y = (y_pos + row) % 32;
                        int idx = pixel_y * 64 + pixel_x;
                        if (cpu->screen[idx] == 1) cpu->V[0xF] = 1;
                        cpu->screen[idx] ^= 1;
                    }
                }
            }
            cpu->dirty = 1;
            break;
        }

        /* ============================================================
           键盘类
           ============================================================ */
        case 0xE000:
            if ((opcode & 0x00FF) == 0x009E) {
                if (cpu->key_down[cpu->V[x]]) cpu->PC += 2;
            } else if ((opcode & 0x00FF) == 0x00A1) {
                if (!cpu->key_down[cpu->V[x]]) cpu->PC += 2;
            }
            break;

        /* ============================================================
           杂项（定时器类 + 等待按键）
           ============================================================ */
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:  /* Fx07: LD Vx, DT */
                    cpu->V[x] = cpu->delay_timer;
                    break;
                case 0x000A:  /* Fx0A: LD Vx, K */
                    cpu->key_waiting = 1;
                    break;
                case 0x0015:  /* Fx15: LD DT, Vx */
                    cpu->delay_timer = cpu->V[x];
                    break;
                case 0x0018:  /* Fx18: LD ST, Vx */
                    cpu->sound_timer = cpu->V[x];
                    break;
                default:
                    break;
            }
            break;

        default:
            printf("未知指令: 0x%04X\n", opcode);
            break;
    }
}