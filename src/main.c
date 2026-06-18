#include "../include/chip8.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("用法: %s <rom文件>\n", argv[0]);
        return 1;
    }

    chip8_t cpu;
    chip8_init(&cpu);

    if (!chip8_load_rom(&cpu, argv[1])) {
        printf("加载 ROM 失败\n");
        return 1;
    }

    printf("===== CHIP-8 模拟器 =====\n");
    printf("PC = 0x%04X\n", cpu.PC);

    // 执行 10 条指令
    for (int i = 0; i < 10; i++) {
        chip8_emulate_cycle(&cpu);
        printf("第 %d 步: PC = 0x%04X\n", i + 1, cpu.PC);
    }

    return 0;
}