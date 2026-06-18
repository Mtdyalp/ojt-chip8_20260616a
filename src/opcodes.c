#include "../include/chip8.h"
#include <stdio.h>

void chip8_execute_opcode(chip8_t *cpu, uint16_t opcode)
{
    /* 暂时只做解码，不执行，所以用不到这些变量 */
    // uint16_t nnn = opcode & 0x0FFF;
    // uint8_t kk = opcode & 0x00FF;
    // uint8_t x = (opcode & 0x0F00) >> 8;

    (void)opcode;  /* 防止编译警告 */
    (void)cpu;     /* 防止编译警告 */

    switch (opcode & 0xF000) {
        case 0x0000:
        case 0x1000:
        case 0x2000:
        case 0x3000:
        case 0x4000:
        case 0x5000:
        case 0x6000:
        case 0x7000:
        case 0x8000:
        case 0x9000:
        case 0xA000:
        case 0xB000:
        case 0xC000:
        case 0xD000:
        case 0xE000:
        case 0xF000:
            /* 占位，什么都不做 */
            break;

        default:
            printf("未知指令: 0x%04X\n", opcode);
            break;
    }
}