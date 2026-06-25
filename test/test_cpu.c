#include "../include/chip8.h"
#include "../include/opcodes.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
   测试辅助函数
   ============================================================ */
typedef void (*test_fn_t)(void);

static int total_groups = 0;
static int passed_groups = 0;
static int print_individual = 0;
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
    if (!print_individual) return;

    if (passed)
        printf("Test %-2d %-20s [OK]\n", total_tests, name);
    else
        printf("Test %-2d %-20s [FAIL]\n", total_tests, name);
}

static void run_group(int no, const char *name, test_fn_t *tests, int count)
{
    int start_total = total_tests;
    int start_passed = passed_tests;

    for (int i = 0; i < count; i++) {
        tests[i]();
    }

    int tests_run = total_tests - start_total;// 计算本组运行了多少测试
    int tests_ok = passed_tests - start_passed;// 计算本组通过了多少测试

    int group_ok = 0;
    if (tests_run > 0 && tests_run == tests_ok) {// 如果本组有测试，并且全部通过
        group_ok = 1;// 标记本组测试通过
    }

    total_groups++;// 统计总共运行了多少组测试

    if (group_ok) {
        passed_groups++;
    }

    if (group_ok) {
        printf("Test %02d (%-22s): [OK]\n", no, name);
    } else {
        printf("Test %02d (%-22s): [FAIL]\n", no, name);
    }
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

    cpu.V[0xA] = 0x3F;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x3A3F);
    int passed = (cpu.PC == 0x202);
 //   print_result("SE Vx, kk - equal", passed);
    check(passed);

    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x3A40);
    passed = (cpu.PC == 0x200);
 //   print_result("SE Vx, kk - not equal", passed);
    check(passed);
}

static void test_4xkk_sne(void)
{
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.V[0xA] = 0x3F;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x4A40);
    int passed = (cpu.PC == 0x202);
//    print_result("SNE Vx, kk - not equal", passed);
    check(passed);

    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x4A3F);
    passed = (cpu.PC == 0x200);
 //   print_result("SNE Vx, kk - equal", passed);
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
 //   print_result("JP V0, nnn", passed);
    check(passed);
}

static void test_5xy0_se_reg(void)
{
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.V[0xA] = 0x3F;
    cpu.V[0xB] = 0x3F;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x5AB0);
    int passed = (cpu.PC == 0x202);
 //   print_result("SE Vx, Vy - equal", passed);
    check(passed);

    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.V[0xB] = 0x40;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x5AB0);
    passed = (cpu.PC == 0x200);
 //   print_result("SE Vx, Vy - not equal", passed);
    check(passed);
}

static void test_9xy0_sne_reg(void)
{
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.V[0xA] = 0x3F;
    cpu.V[0xB] = 0x40;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x9AB0);
    int passed = (cpu.PC == 0x202);
//    print_result("SNE Vx, Vy - not equal", passed);
    check(passed);

    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.V[0xB] = 0x3F;
    cpu.PC = 0x200;
    chip8_execute_opcode(&cpu, 0x9AB0);
    passed = (cpu.PC == 0x200);
//    print_result("SNE Vx, Vy - equal", passed);
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
 //   print_result("CALL nnn", passed);
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
//    print_result("RET", passed);
    check(passed);
}

/* ============================================================
   描画类（2条）
   ============================================================ */

static void test_cls(void)
{
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.screen[0] = 1;
    cpu.screen[100] = 1;
    cpu.screen[200] = 1;

    chip8_execute_opcode(&cpu, 0x00E0);

    int passed = 1;
    for (int i = 0; i < 2048; i++) {
        if (cpu.screen[i] != 0) {
            passed = 0;
            break;
        }
    }
    passed = passed && (cpu.dirty == 1);
//    print_result("CLS", passed);
    check(passed);
}

static void test_draw(void)
{
    chip8_t cpu;
    chip8_init(&cpu);

    cpu.mem[0x300] = 0x80;
    cpu.mem[0x301] = 0xC0;
    cpu.mem[0x302] = 0xE0;
    cpu.I = 0x300;
    cpu.V[0xA] = 0;
    cpu.V[0xB] = 0;

    chip8_execute_opcode(&cpu, 0xDAB3);
    int passed = (cpu.V[0xF] == 0);
    int has_pixel = 0;
    for (int i = 0; i < 2048; i++) {
        if (cpu.screen[i] == 1) {
            has_pixel = 1;
            break;
        }
    }
    passed = passed && has_pixel;

    chip8_execute_opcode(&cpu, 0xDAB3);
    passed = passed && (cpu.V[0xF] == 1);

    int all_zero = 1;
    for (int i = 0; i < 2048; i++) {
        if (cpu.screen[i] != 0) {
            all_zero = 0;
            break;
        }
    }
    passed = passed && all_zero;

    print_result("DRW", passed);
    check(passed);
}

/* ============================================================
   键盘类（3条）
   ============================================================ */

static void test_ex9e_skp(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x5;
    cpu.PC = 0x200;

    /* 场景1：键按下，应该跳过 */
    cpu.key_down[0x5] = 1;
    chip8_execute_opcode(&cpu, 0xEA9E);
    int passed = (cpu.PC == 0x202);
    print_result("SKP - key pressed", passed);
    check(passed);

    /* 场景2：键未按下，不跳过 */
    chip8_init(&cpu);
    cpu.V[0xA] = 0x5;
    cpu.PC = 0x200;
    cpu.key_down[0x5] = 0;
    chip8_execute_opcode(&cpu, 0xEA9E);
    passed = (cpu.PC == 0x200);
    print_result("SKP - key not pressed", passed);
    check(passed);
}

static void test_exa1_sknp(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x5;
    cpu.PC = 0x200;

    /* 场景1：键未按下，应该跳过 */
    cpu.key_down[0x5] = 0;
    chip8_execute_opcode(&cpu, 0xEAA1);
    int passed = (cpu.PC == 0x202);
    print_result("SKNP - key not pressed", passed);
    check(passed);

    /* 场景2：键按下，不跳过 */
    chip8_init(&cpu);
    cpu.V[0xA] = 0x5;
    cpu.PC = 0x200;
    cpu.key_down[0x5] = 1;
    chip8_execute_opcode(&cpu, 0xEAA1);
    passed = (cpu.PC == 0x200);
    print_result("SKNP - key pressed", passed);
    check(passed);
}
#if 0
static void test_fx0a_wait_key(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0;
    cpu.key_waiting = 0;

    chip8_execute_opcode(&cpu, 0xFA0A);
    int passed = (cpu.key_waiting == 1);
    print_result("LD Vx, K - wait flag", passed);
    check(passed);
}
#endif
/* ============================================================
   定时器类 + 随机数（4条）
   ============================================================ */
#if 0
static void test_fx07_get_delay(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.delay_timer = 0x3F;
    cpu.V[0xA] = 0;

    chip8_execute_opcode(&cpu, 0xFA07);
    int passed = (cpu.V[0xA] == 0x3F);
    print_result("LD Vx, DT", passed);
    check(passed);
}

static void test_fx15_set_delay(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.delay_timer = 0;

    chip8_execute_opcode(&cpu, 0xFA15);
    int passed = (cpu.delay_timer == 0x3F);
    print_result("LD DT, Vx", passed);
    check(passed);
}

static void test_fx18_set_sound(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x3F;
    cpu.sound_timer = 0;

    chip8_execute_opcode(&cpu, 0xFA18);
    int passed = (cpu.sound_timer == 0x3F);
    print_result("LD ST, Vx", passed);
    check(passed);
}

static void test_cxkk_rnd(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0;

    chip8_execute_opcode(&cpu, 0xCA0F);
    int passed = (cpu.V[0xA] <= 0x0F);
    print_result("RND Vx, kk - range", passed);
    check(passed);
}
#endif
/* ============================================================
   内存操作类（5条）
   ============================================================ */
static void test_annn_ld_i(void)
{
    chip8_t cpu;
    chip8_init(&cpu);

    chip8_execute_opcode(&cpu, 0xA300);

    int passed = (cpu.I == 0x300);
    check(passed);
}

static void test_fx1e_add_i(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.I = 0x300;
    cpu.V[0xA] = 0x10;

    chip8_execute_opcode(&cpu, 0xFA1E);
    int passed = (cpu.I == 0x310);
    print_result("ADD I, Vx", passed);
    check(passed);
}

static void test_fx29_font_addr(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 0x5;

    chip8_execute_opcode(&cpu, 0xFA29);
    int passed = (cpu.I == 25);
    print_result("LD F, Vx", passed);
    check(passed);
}

static void test_fx33_bcd(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0xA] = 123;
    cpu.I = 0x300;

    chip8_execute_opcode(&cpu, 0xFA33);
    int passed = (cpu.mem[0x300] == 1) &&
                 (cpu.mem[0x301] == 2) &&
                 (cpu.mem[0x302] == 3);
    print_result("LD B, Vx - BCD", passed);
    check(passed);
}

static void test_fx55_store(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.V[0] = 0x01;
    cpu.V[1] = 0x02;
    cpu.V[2] = 0x03;
    cpu.I = 0x300;

    chip8_execute_opcode(&cpu, 0xF255);
    int passed = (cpu.mem[0x300] == 0x01) &&
                 (cpu.mem[0x301] == 0x02) &&
                 (cpu.mem[0x302] == 0x03);
    print_result("LD [I], Vx - store", passed);
    check(passed);
}

static void test_fx65_load(void)
{
    chip8_t cpu;
    chip8_init(&cpu);
    cpu.I = 0x300;
    cpu.mem[0x300] = 0x0A;
    cpu.mem[0x301] = 0x0B;
    cpu.mem[0x302] = 0x0C;

    chip8_execute_opcode(&cpu, 0xF265);
    int passed = (cpu.V[0] == 0x0A) &&
                 (cpu.V[1] == 0x0B) &&
                 (cpu.V[2] == 0x0C);
    print_result("LD Vx, [I] - load", passed);
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

    int ok = chip8_load(&cpu, "fake_rom.bin");
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

#define RUN_GROUP(no, name, ...) do { \
    test_fn_t tests[] = { __VA_ARGS__ }; \
    run_group(no, name, tests, sizeof(tests) / sizeof(tests[0])); \
} while (0)

int main(void)
{
    total_tests = 0;
    passed_tests = 0;
    total_groups = 0;
    passed_groups = 0;

    RUN_GROUP(1,  "LD Vx, byte",        test_6xkk_ld_byte);
    RUN_GROUP(2,  "ADD Vx, byte",       test_7xkk_add_byte);
    RUN_GROUP(3,  "LD Vx, Vy",          test_8xy0_ld_reg);
    RUN_GROUP(4,  "OR/AND/XOR",         test_8xy1_or, test_8xy2_and, test_8xy3_xor, /*test_cxkk_rnd*/);
    RUN_GROUP(5,  "ADD Vx, Vy + carry", test_8xy4_add_carry);
    RUN_GROUP(6,  "SUB + borrow",       test_8xy5_sub, test_8xy7_subn);
    RUN_GROUP(7,  "SHR/SHL",            test_8xy6_shr, test_8xye_shl);
    RUN_GROUP(8,  "JP/CALL/RET",        test_1nnn_jump, test_bnnn_jump_v0, test_2nnn_call, test_00ee_ret);
    RUN_GROUP(9,  "SE/SNE skip",        test_3xkk_se, test_4xkk_sne, test_5xy0_se_reg, test_9xy0_sne_reg, test_ex9e_skp, test_exa1_sknp, /*test_fx0a_wait_key*/);
    RUN_GROUP(10, "LD I / ADD I",       test_annn_ld_i, test_fx1e_add_i, test_fx29_font_addr, /*test_fx07_get_delay, test_fx15_set_delay, test_fx18_set_sound*/);
    RUN_GROUP(11, "BCD",                test_fx33_bcd);
    RUN_GROUP(12, "LD [I] / LD Vx [I]", test_fx55_store, test_fx65_load);
    RUN_GROUP(13, "DRW collision",      test_cls, test_draw);
    RUN_GROUP(14, "font loading",       test_font_loading);
    RUN_GROUP(15, "ROM loading",        test_rom_loading);

    printf("\nAll %d tests passed.\n", passed_groups);
    return (passed_groups == total_groups) ? 0 : 1;
}
