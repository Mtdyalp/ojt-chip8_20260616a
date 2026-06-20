#include "../include/chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void chip8_execute_opcode(chip8_t *cpu, uint16_t opcode)
{
    // uint16_t nnn = opcode & 0x0FFF;   /* 第2步才用到，暂时注释 */
    uint8_t kk = opcode & 0x00FF;
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = opcode & 0x000F;

    switch (opcode & 0xF000) {
        /* ============================================================
           0x0000: 系统类（后续实现）
           ============================================================ */
        case 0x0000:
            break;

        /* ============================================================
           0x1000: 控制流类（后续实现）
           ============================================================ */
        case 0x1000:
            break;

        case 0x2000:
            break;

        case 0x3000:
            break;

        case 0x4000:
            break;

        case 0x5000:
            break;

        /* ============================================================
           第1步：寄存器操作类（11条）✅
           ============================================================ */

        case 0x6000:  /* 6xkk: LD Vx, kk */
            cpu->V[x] = kk;
            break;

        case 0x7000:  /* 7xkk: ADD Vx, kk */
            cpu->V[x] += kk;
            break;

        case 0x8000:  /* 8xyn: 寄存器操作族 */
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
                    printf("未知 8XYN 操作: 0x%X\n", n);
                    break;
            }
            break;

        /* ============================================================
           0x9000: 控制流类（后续实现）
           ============================================================ */
        case 0x9000:
            break;

        /* ============================================================
           0xA000: 内存操作类（后续实现）
           ============================================================ */
        case 0xA000:
            break;

        case 0xB000:
            break;

        case 0xC000:
            break;

        /* ============================================================
           0xD000: 绘制类（后续实现）
           ============================================================ */
        case 0xD000:
            break;

        /* ============================================================
           0xE000: 键盘类（后续实现）
           ============================================================ */
        case 0xE000:
            break;

        /* ============================================================
           0xF000: 杂项（后续实现）
           ============================================================ */
        case 0xF000:
            break;

        default:
            printf("未知指令: 0x%04X\n", opcode);
            break;
    }
}