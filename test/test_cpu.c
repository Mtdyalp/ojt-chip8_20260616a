#include "../include/chip8.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
   测试辅助函数
   ============================================================ */

static int total_tests = 0;
static int passed_tests = 0;

static void check(int passed)
{
    total_tests++;
    if (passed) {
        passed_tests++;
    }
}

static void print_result(const char *name, int passed)
{
    if (passed)
        printf("Test %-2d %-20s [OK]\n", total_tests, name);
    else
        printf("Test %-2d %-20s [FAIL]\n", total_tests, name);
}

/* ============================================================
   测试1：6xkk - LD Vx, kk
   ============================================================ */

static void test_6xkk_ld_byte(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    chip8_execute_opcode(&cpu, 0x6A3F);
    int passed = (cpu.V[0xA] == 0x3F);
    print_result("LD Vx, byte", passed);
    check(passed);
}

/* ============================================================
   测试2：7xkk - ADD Vx, kk
   ============================================================ */

static void test_7xkk_add_byte(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x10;
    chip8_execute_opcode(&cpu, 0x7A05);
    int passed = (cpu.V[0xA] == 0x15);
    print_result("ADD Vx, byte", passed);
    check(passed);
}

/* ============================================================
   测试3：8xy0 - LD Vx, Vy
   ============================================================ */

static void test_8xy0_ld_reg(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xB] = 0x3F;
    chip8_execute_opcode(&cpu, 0x8AB0);
    int passed = (cpu.V[0xA] == 0x3F);
    print_result("LD Vx, Vy", passed);
    check(passed);
}

/* ============================================================
   测试4-6：8xy1 OR / 8xy2 AND / 8xy3 XOR
   ============================================================ */

static void test_8xy1_or(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.V[0xB] = 0x80;
    chip8_execute_opcode(&cpu, 0x8AB1);
    int passed = (cpu.V[0xA] == 0xBF) && (cpu.V[0xF] == 0);
    print_result("OR/AND/XOR", passed);
    check(passed);
}

static void test_8xy2_and(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.V[0xB] = 0x80;
    chip8_execute_opcode(&cpu, 0x8AB2);
    int passed = (cpu.V[0xA] == 0x00) && (cpu.V[0xF] == 0);
    print_result("OR/AND/XOR", passed);
    check(passed);
}

static void test_8xy3_xor(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.V[0xB] = 0x80;
    chip8_execute_opcode(&cpu, 0x8AB3);
    int passed = (cpu.V[0xA] == 0xBF) && (cpu.V[0xF] == 0);
    print_result("OR/AND/XOR", passed);
    check(passed);
}

/* ============================================================
   测试7：8xy4 - ADD Vx, Vy (带进位)
   ============================================================ */

static void test_8xy4_add_carry(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0xF0;
    cpu.V[0xB] = 0x20;
    chip8_execute_opcode(&cpu, 0x8AB4);
    int passed = (cpu.V[0xA] == 0x10) && (cpu.V[0xF] == 1);
    print_result("ADD Vx, Vy + carry", passed);
    check(passed);
}

/* ============================================================
   测试8：8xy5 - SUB Vx, Vy
   ============================================================ */

static void test_8xy5_sub(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x50;
    cpu.V[0xB] = 0x20;
    chip8_execute_opcode(&cpu, 0x8AB5);
    int passed = (cpu.V[0xA] == 0x30) && (cpu.V[0xF] == 1);
    print_result("SUB + borrow", passed);
    check(passed);
}

/* ============================================================
   测试9：8xy6 - SHR Vx
   ============================================================ */

static void test_8xy6_shr(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x85;
    chip8_execute_opcode(&cpu, 0x8A06);
    int passed = (cpu.V[0xA] == 0x42) && (cpu.V[0xF] == 1);
    print_result("SHR/SHL", passed);
    check(passed);
}

/* ============================================================
   测试10：8xy7 - SUBN Vx, Vy
   ============================================================ */

static void test_8xy7_subn(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x20;
    cpu.V[0xB] = 0x50;
    chip8_execute_opcode(&cpu, 0x8AB7);
    int passed = (cpu.V[0xA] == 0x30) && (cpu.V[0xF] == 1);
    print_result("SUB + borrow", passed);
    check(passed);
}

/* ============================================================
   测试11：8xyE - SHL Vx
   ============================================================ */

static void test_8xye_shl(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x85;
    chip8_execute_opcode(&cpu, 0x8A0E);
    int passed = (cpu.V[0xA] == 0x0A) && (cpu.V[0xF] == 1);
    print_result("SHR/SHL", passed);
    check(passed);
}

/* ============================================================
   测试12：font loading
   ============================================================ */

static void test_font_loading(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    int passed = (cpu.mem[0x000] == 0xF0) &&
                 (cpu.mem[0x004] == 0xF0) &&
                 (cpu.mem[0x005] == 0x20);
    print_result("font loading", passed);
    check(passed);
}

/* ============================================================
   测试13：ROM loading
   ============================================================ */

static void test_rom_loading(void)
{
    chip8_t cpu;
    chip8_init(&cpu);

    uint8_t fake[] = {0xA0, 0x12, 0x6B, 0xFF};
    FILE *fp = fopen("fake_rom.bin", "wb");
    fwrite(fake, 1, sizeof(fake), fp);
    fclose(fp);

    int ok = chip8_load_rom(&cpu, "fake_rom.bin");
    int passed = (ok == 1) &&
                 (cpu.mem[0x200] == 0xA0) &&
                 (cpu.mem[0x201] == 0x12);

    remove("fake_rom.bin");
    print_result("ROM loading", passed);
    check(passed);
}

/* ============================================================
   main
   ============================================================ */

int main(void)
{
    printf("\n");
    total_tests = 0;
    passed_tests = 0;

    test_6xkk_ld_byte();
    test_7xkk_add_byte();
    test_8xy0_ld_reg();
    test_8xy1_or();
    test_8xy2_and();
    test_8xy3_xor();
    test_8xy4_add_carry();
    test_8xy5_sub();
    test_8xy6_shr();
    test_8xy7_subn();
    test_8xye_shl();
    test_font_loading();
    test_rom_loading();

    printf("\nAll %d tests passed.\n", passed_tests);
    return (passed_tests == total_tests) ? 0 : 1;
}