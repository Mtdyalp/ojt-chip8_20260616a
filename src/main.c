#include "../include/chip8.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("用法: %s <rom文件>\n", argv[0]);
        printf("示例: %s roms/pong.ch8\n", argv[0]);
        return 1;
    }

    chip8_t cpu;
    chip8_init(&cpu);

    if (!chip8_load(&cpu, argv[1])) {
        printf("加载 ROM 失败\n");
        return 1;
    }

    printf("===== CHIP-8 模拟器 =====\n");
    printf("ROM: %s\n", argv[1]);
    printf("PC = 0x%04X\n", cpu.PC);

    // 执行 1000 条指令（足够看到画面）
    for (int i = 0; i < 1000; i++) {
        chip8_emulate_cycle(&cpu);
        
        // 如果 PC 超出 ROM 范围或跑到 0x0000，停止
        if (cpu.PC == 0x0000 || cpu.PC >= 0xFFF) {
            printf("程序结束或出错，PC = 0x%04X\n", cpu.PC);
            break;
        }
    }

    printf("执行完毕\n");
    return 0;
}
