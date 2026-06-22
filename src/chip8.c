#include "../include/chip8.h"
#include "../include/opcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------
   字库：16 个字符，每个 5 字节
   从 0x000 开始放
   ------------------------------------------------------------ */
static const uint8_t font_data[80] = {
    0xF0,0x90,0x90,0x90,0xF0,
    0x20,0x60,0x20,0x20,0x70,
    0xF0,0x10,0xF0,0x80,0xF0,
    0xF0,0x10,0xF0,0x10,0xF0,
    0x90,0x90,0xF0,0x10,0x10,
    0xF0,0x80,0xF0,0x10,0xF0,
    0xF0,0x80,0xF0,0x90,0xF0,
    0xF0,0x10,0x20,0x40,0x40,
    0xF0,0x90,0xF0,0x90,0xF0,
    0xF0,0x90,0xF0,0x10,0xF0,
    0xF0,0x90,0xF0,0x90,0x90,
    0xE0,0x90,0xE0,0x90,0xE0,
    0xF0,0x80,0x80,0x80,0xF0,
    0xE0,0x90,0x90,0x90,0xE0,
    0xF0,0x80,0xF0,0x80,0xF0,
    0xF0,0x80,0xF0,0x80,0x80
};

/* ------------------------------------------------------------
   初始化：清零 + 灌字库
   ------------------------------------------------------------ */
void chip8_init(chip8_t *cpu)
{
    memset(cpu, 0, sizeof(chip8_t));
    memcpy(cpu->mem, font_data, sizeof(font_data));
    cpu->PC = 0x200;
    cpu->running = 1;
    cpu->dirty = 1;
}

/* ------------------------------------------------------------
   加载 ROM：读到 0x200 位置
   ------------------------------------------------------------ */
int chip8_load(chip8_t *cpu, const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "chip8: 打不开 %s\n", path);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (sz > 4096 - 0x200) {
        fprintf(stderr, "chip8: ROM 太大 (%ld 字节)\n", sz);
        fclose(fp);
        return 0;
    }

    size_t n = fread(cpu->mem + 0x200, 1, sz, fp);
    fclose(fp);

    if (n != (size_t)sz) {
        fprintf(stderr, "chip8: 读取不完整 (%zu / %ld)\n", n, sz);
        return 0;
    }

    printf("chip8: 加载 %s 成功 (%ld 字节)\n", path, sz);
    return 1;
}

void chip8_emulate_cycle(chip8_t *cpu)
{
    /* wait key: skip fetch/decode, poll keys only */
    if (cpu->key_waiting) {
        for (int i = 0; i < 16; i++) {
            if (cpu->key_down[i]) {
                cpu->V[cpu->key_wait_value] = i;
                cpu->key_waiting = 0;
                break;
            }
        }
        goto timers;
    }

    uint16_t op = (cpu->mem[cpu->PC] << 8) | cpu->mem[cpu->PC + 1];
    cpu->PC += 2;
    chip8_execute_opcode(cpu, op);

timers:
    /* 60Hz timer decrement */
    if (cpu->delay_timer > 0) cpu->delay_timer--;
    if (cpu->sound_timer > 0) cpu->sound_timer--;
}

void chip8_execute_opcode(chip8_t *cpu, uint16_t op)
{
    switch (op & 0xF000) {
        case 0x0000:
            if (op == 0x00E0) op_00e0(cpu, op);
            else if (op == 0x00EE) op_00ee(cpu, op);
            break;
        case 0x1000: op_1nnn(cpu, op); break;
        case 0x2000: op_2nnn(cpu, op); break;
        case 0x3000: op_3xkk(cpu, op); break;
        case 0x4000: op_4xkk(cpu, op); break;
        case 0x5000: op_5xy0(cpu, op); break;
        case 0x6000: op_6xkk(cpu, op); break;
        case 0x7000: op_7xkk(cpu, op); break;
        case 0x8000:
            switch (N(op)) {
                case 0x0: op_8xy0(cpu, op); break;
                case 0x1: op_8xy1(cpu, op); break;
                case 0x2: op_8xy2(cpu, op); break;
                case 0x3: op_8xy3(cpu, op); break;
                case 0x4: op_8xy4(cpu, op); break;
                case 0x5: op_8xy5(cpu, op); break;
                case 0x6: op_8xy6(cpu, op); break;
                case 0x7: op_8xy7(cpu, op); break;
                case 0xE: op_8xye(cpu, op); break;
                default: break;
            }
            break;
        case 0x9000: op_9xy0(cpu, op); break;
        case 0xA000: op_annn(cpu, op); break;
        case 0xB000: op_bnnn(cpu, op); break;
        case 0xC000: op_cxkk(cpu, op); break;
        case 0xD000: op_dxyn(cpu, op); break;
        case 0xE000:
            if ((op & 0x00FF) == 0x009E) op_ex9e(cpu, op);
            else if ((op & 0x00FF) == 0x00A1) op_exa1(cpu, op);
            break;
        case 0xF000:
            switch (op & 0x00FF) {
                case 0x07: op_fx07(cpu, op); break;
                case 0x0A: op_fx0a(cpu, op); break;
                case 0x15: op_fx15(cpu, op); break;
                case 0x18: op_fx18(cpu, op); break;
                case 0x1E: op_fx1e(cpu, op); break;
                case 0x29: op_fx29(cpu, op); break;
                case 0x33: op_fx33(cpu, op); break;
                case 0x55: op_fx55(cpu, op); break;
                case 0x65: op_fx65(cpu, op); break;
                default: break;
            }
            break;
        default:
            printf("Unknown opcode: 0x%04X\n", op);
            break;
    }
}

