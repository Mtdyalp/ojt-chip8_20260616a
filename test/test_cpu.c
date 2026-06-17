#include "../include/chip8.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------
   测试辅助
   ------------------------------------------------------------ */

static void check(const char *name, int want, int got)
{
    if (want == got)
        printf("  [OK] %s\n", name);
    else
        printf("  [FAIL] %s (期望 %d, 实际 %d)\n", name, want, got);
}

/* ------------------------------------------------------------
   测试用例
   ------------------------------------------------------------ */

static void test_init(void)
{
    printf("\n[测试] 初始化 + 字库加载\n");

    chip8_t cpu;
    chip8_init(&cpu);

    check("mem[0x000] == 0xF0",  0xF0, cpu.mem[0x000]);
    check("mem[0x004] == 0xF0",  0xF0, cpu.mem[0x004]);
    check("mem[0x005] == 0x20",  0x20, cpu.mem[0x005]);
    check("mem[0x04C] == 0x80",  0x80, cpu.mem[0x04C]);  /* 改成 0x04C */
    check("mem[0x04F] == 0x80",  0x80, cpu.mem[0x04F]);
    check("PC == 0x200",         0x200, cpu.PC);
    check("V[0] == 0",           0, cpu.V[0]);
    check("SP == 0",             0, cpu.SP);
}

static void test_load_rom(void)
{
    printf("\n[测试] ROM 加载到 0x200\n");

    chip8_t cpu;
    chip8_init(&cpu);

    uint8_t fake[] = {0xA0, 0x12, 0x6B, 0xFF};
    FILE *fp = fopen("fake_rom.bin", "wb");
    fwrite(fake, 1, sizeof(fake), fp);
    fclose(fp);

    int ok = chip8_load_rom(&cpu, "fake_rom.bin");
    check("加载返回成功", 1, ok);

    check("mem[0x200] == 0xA0", 0xA0, cpu.mem[0x200]);
    check("mem[0x201] == 0x12", 0x12, cpu.mem[0x201]);
    check("mem[0x202] == 0x6B", 0x6B, cpu.mem[0x202]);
    check("mem[0x203] == 0xFF", 0xFF, cpu.mem[0x203]);

    remove("fake_rom.bin");
}

int main(void)
{
    printf("===== CHIP-8 第一阶段测试 =====\n");
    test_init();
    test_load_rom();
    printf("\n[完成] 部分测试跑完\n");
    return 0;
}