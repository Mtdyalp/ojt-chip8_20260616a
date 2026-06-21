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
                /* CLS 清屏 */
                memset(cpu->screen, 0, sizeof(cpu->screen));
                cpu->dirty = 1;
            } else if (opcode == 0x00EE) {
                /* 00EE: RET - 从子程序返回 */
                if (cpu->SP > 0) {
                    cpu->SP--;
                    cpu->PC = cpu->stack[cpu->SP];
                }
            }
            break;

        /* ============================================================
           控制流类
           ============================================================ */
        case 0x1000:  /* 1nnn: JP nnn */
            cpu->PC = nnn;
            break;

        case 0x2000:  /* 2nnn: CALL nnn */
            if (cpu->SP < 16) {
                cpu->stack[cpu->SP] = cpu->PC;
                cpu->SP++;
                cpu->PC = nnn;
            }
            break;

        case 0x3000:  /* 3xkk: SE Vx, kk */
            if (cpu->V[x] == kk) {
                cpu->PC += 2;
            }
            break;

        case 0x4000:  /* 4xkk: SNE Vx, kk */
            if (cpu->V[x] != kk) {
                cpu->PC += 2;
            }
            break;

        case 0x5000:  /* 5xy0: SE Vx, Vy */
            if (cpu->V[x] == cpu->V[y]) {
                cpu->PC += 2;
            }
            break;

        case 0x9000:  /* 9xy0: SNE Vx, Vy */
            if (cpu->V[x] != cpu->V[y]) {
                cpu->PC += 2;
            }
            break;

        case 0xB000:  /* Bnnn: JP V0, nnn */
            cpu->PC = nnn + cpu->V[0];
            break;

        /* ============================================================
           寄存器操作类
           ============================================================ */
        case 0x6000:  /* 6xkk: LD Vx, kk */
            cpu->V[x] = kk;
            break;

        case 0x7000:  /* 7xkk: ADD Vx, kk */
            cpu->V[x] += kk;
            break;

        case 0x8000:
            switch (n) {
                case 0x0:  /* 8xy0: LD Vx, Vy */
                    cpu->V[x] = cpu->V[y];
                    break;

                case 0x1:  /* 8xy1: OR Vx, Vy */
                    cpu->V[x] |= cpu->V[y];
                    cpu->V[0xF] = 0;
                    break;

                case 0x2:  /* 8xy2: AND Vx, Vy */
                    cpu->V[x] &= cpu->V[y];
                    cpu->V[0xF] = 0;
                    break;

                case 0x3:  /* 8xy3: XOR Vx, Vy */
                    cpu->V[x] ^= cpu->V[y];
                    cpu->V[0xF] = 0;
                    break;

                case 0x4:  /* 8xy4: ADD Vx, Vy (带进位) */
                {
                    uint16_t result = cpu->V[x] + cpu->V[y];
                    cpu->V[0xF] = (result > 0xFF) ? 1 : 0;
                    cpu->V[x] = result & 0xFF;
                    break;
                }

                case 0x5:  /* 8xy5: SUB Vx, Vy */
                    cpu->V[0xF] = (cpu->V[x] >= cpu->V[y]) ? 1 : 0;
                    cpu->V[x] -= cpu->V[y];
                    break;

                case 0x6:  /* 8xy6: SHR Vx */
                    cpu->V[0xF] = cpu->V[x] & 0x01;
                    cpu->V[x] >>= 1;
                    break;

                case 0x7:  /* 8xy7: SUBN Vx, Vy */
                    cpu->V[0xF] = (cpu->V[y] >= cpu->V[x]) ? 1 : 0;
                    cpu->V[x] = cpu->V[y] - cpu->V[x];
                    break;

                case 0xE:  /* 8xyE: SHL Vx */
                    cpu->V[0xF] = (cpu->V[x] & 0x80) ? 1 : 0;
                    cpu->V[x] <<= 1;
                    break;

                default:
                    break;
            }
            break;

        /* ============================================================
           内存操作类（部分）
           ============================================================ */
        case 0xA000:  /* Annn: LD I, nnn */
            cpu->I = nnn;
            break;

        /* ============================================================
           描画类
           ============================================================ */
        case 0xD000:  /* Dxyn: DRW Vx, Vy, n */
        {
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
                        if (cpu->screen[idx] == 1) {
                            cpu->V[0xF] = 1;
                        }
                        cpu->screen[idx] ^= 1;
                    }
                }
            }
            cpu->dirty = 1;
            break;
        }

        /* ============================================================
           键盘类（新增）
           ============================================================ */
        case 0xE000:
            if ((opcode & 0x00FF) == 0x009E) {
                /* Ex9E: SKP Vx - 按下则跳过 */
                if (cpu->key_down[cpu->V[x]]) {
                    cpu->PC += 2;
                }
            } else if ((opcode & 0x00FF) == 0x00A1) {
                /* ExA1: SKNP Vx - 未按则跳过 */
                if (!cpu->key_down[cpu->V[x]]) {
                    cpu->PC += 2;
                }
            }
            break;

        /* ============================================================
           杂项（定时器、随机数、内存操作等）
           ============================================================ */
        case 0xC000:  /* Cxkk: RND Vx, kk - 随机数（先占位） */
            break;

        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x000A:  /* Fx0A: LD Vx, K - 等待按键 */
                    cpu->key_waiting = 1;
                    break;
                /* 其他 Fx 指令稍后实现 */
                default:
                    break;
            }
            break;

        default:
            printf("未知指令: 0x%04X\n", opcode);
            break;
    }
}