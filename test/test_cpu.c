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
   寄存器操作类（11条）
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
   控制流类（8条）
   ============================================================ */

static void test_1nnn_jump(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x1A35);
    int passed = (cpu.PC == 0xA35);
    print_result("JP nnn", passed);
    check(passed);
}

static void test_3xkk_se(void)
{
    chip8_t cpu;
    chip8_init(&cpu);

    /* 条件成立：相等，跳过 */
    cpu.V[0xA] = 0x3F;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x3A3F);
    int passed = (cpu.PC == 0x202);
    print_result("SE Vx, kk - equal", passed);
    check(passed);

    /* 条件不成立：不相等，不跳过 */
    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x3A40);
    passed = (cpu.PC == 0x200);
    print_result("SE Vx, kk - not equal", passed);
    check(passed);
}

static void test_4xkk_sne(void)
{
    chip8_t cpu;
    chip8_init(&cpu);

    /* 条件成立：不相等，跳过 */
    cpu.V[0xA] = 0x3F;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x4A40);
    int passed = (cpu.PC == 0x202);
    print_result("SNE Vx, kk - not equal", passed);
    check(passed);

    /* 条件不成立：相等，不跳过 */
    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x4A3F);
    passed = (cpu.PC == 0x200);
    print_result("SNE Vx, kk - equal", passed);
    check(passed);
}

static void test_bnnn_jump_v0(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0] = 0x10;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0xBA35);
    int passed = (cpu.PC == 0xA45);
    print_result("JP V0, nnn", passed);
    check(passed);
}

static void test_5xy0_se_reg(void)
{
    chip8_t cpu;
    chip8_init(&cpu);

    /* 条件成立：相等，跳过 */
    cpu.V[0xA] = 0x3F;
    cpu.V[0xB] = 0x3F;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x5AB0);
    int passed = (cpu.PC == 0x202);
    print_result("SE Vx, Vy - equal", passed);
    check(passed);

    /* 条件不成立：不相等，不跳过 */
    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.V[0xB] = 0x40;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x5AB0);
    passed = (cpu.PC == 0x200);
    print_result("SE Vx, Vy - not equal", passed);
    check(passed);
}

static void test_9xy0_sne_reg(void)
{
    chip8_t cpu;
    chip8_init(&cpu);

    /* 条件成立：不相等，跳过 */
    cpu.V[0xA] = 0x3F;
    cpu.V[0xB] = 0x40;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x9AB0);
    int passed = (cpu.PC == 0x202);
    print_result("SNE Vx, Vy - not equal", passed);
    check(passed);

    /* 条件不成立：相等，不跳过 */
    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.V[0xB] = 0x3F;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x9AB0);
    passed = (cpu.PC == 0x200);
    print_result("SNE Vx, Vy - equal", passed);
    check(passed);
}

static void test_2nnn_call(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x2A35);

    int passed = (cpu.SP == 1) &&
                 (cpu.stack[0] == 0x200) &&
                 (cpu.PC == 0xA35);
    print_result("CALL nnn", passed);
    check(passed);
}

static void test_00ee_ret(void)
{
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x2A35);
    chip8_execute_opcode(&cpu, 0x00EE);

    int passed = (cpu.SP == 0) &&
                 (cpu.PC == 0x200);
    print_result("RET", passed);
    check(passed);
}

/* ============================================================
   基础功能
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

    /* 寄存器操作类 (11条) */
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

    /* 控制流类 (8条) */
    test_1nnn_jump();
    test_3xkk_se();
    test_4xkk_sne();
    test_bnnn_jump_v0();
    test_5xy0_se_reg();
    test_9xy0_sne_reg();
    test_2nnn_call();
    test_00ee_ret();

    /* 基础功能 (2条) */
    test_font_loading();
    test_rom_loading();

    printf("\nAll %d tests passed.\n", passed_tests);
    return (passed_tests == total_tests) ? 0 : 1;
}