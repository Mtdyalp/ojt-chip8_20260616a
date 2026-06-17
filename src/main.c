#include "../include/chip8.h"
#include <stdio.h>
#include <stdlib.h>

/* ------------------------------------------------------------
   打印内存区域（调试用）
   ------------------------------------------------------------ */

static void dump_mem(const uint8_t *mem, int start, int len)
{
    for (int i = 0; i < len; i++) {
        printf("%02X ", mem[start + i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    if (len % 16 != 0)
        printf("\n");
}

/* ------------------------------------------------------------
   打印字库（前 80 字节）
   ------------------------------------------------------------ */

static void show_font(const chip8_t *cpu)
{
    printf("\n[字库] 0x000 ~ 0x04F:\n");
    dump_mem(cpu->mem, 0x000, 80);
}

/* ------------------------------------------------------------
   打印 ROM 内容（前 64 字节）
   ------------------------------------------------------------ */

static void show_rom(const chip8_t *cpu)
{
    printf("\n[ROM] 0x200 ~ 0x23F:\n");
    dump_mem(cpu->mem, CHIP8_PROG_START, 64);
}

/* ------------------------------------------------------------
   打印 CPU 初始状态
   ------------------------------------------------------------ */

static void show_status(const chip8_t *cpu)
{
    printf("\n[状态]\n");
    printf("  PC = 0x%04X\n", cpu->PC);
    printf("  SP = %d\n", cpu->SP);
    printf("  I  = 0x%04X\n", cpu->I);
    printf("  V0 = 0x%02X\n", cpu->V[0]);
    printf("  VF = 0x%02X\n", cpu->V[15]);
    printf("  running = %d\n", cpu->running);
}

/* ------------------------------------------------------------
   main
   ------------------------------------------------------------ */

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "用法: %s <rom_file>\n", argv[0]);
        return 1;
    }

    chip8_t cpu;
    chip8_init(&cpu);

    printf("===== CHIP-8 模拟器 (调试模式) =====\n");

    if (!chip8_load_rom(&cpu, argv[1])) {
        fprintf(stderr, "加载 ROM 失败，退出\n");
        return 1;
    }

    show_status(&cpu);
    show_font(&cpu);
    show_rom(&cpu);

    printf("\n[完成] ROM 已加载，按任意键退出...\n");
    getchar();

    return 0;
}