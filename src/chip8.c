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

    size_t n = fread(cpu->mem + 0x200, 1, sz, fp);//从 ROM 文件读取数据到内存
    fclose(fp);

    if (n != (size_t)sz) {
        fprintf(stderr, "chip8: 读取不完整 (%zu / %ld)\n", n, sz);
        return 0;
    }

 //   printf("chip8: 加载 %s 成功 (%ld 字节)\n", path, sz);
    return 1;
}

void chip8_emulate_cycle(chip8_t *cpu)
{
 #if 1     
      // 1. FETCH：从内存读 2 字节
       uint16_t op = (cpu->mem[cpu->PC] << 8) | cpu->mem[cpu->PC + 1];
       cpu->PC += 2;
       // 2. EXECUTE：解码并执行
       chip8_execute_opcode(cpu, op);

 #else
    /* 如果 CPU 正在等按键，就先不要执行下一条 CHIP-8 指令*/
    if (cpu->key_waiting) {// 等待按键
        for (int i = 0; i < 16; i++) {
            if (cpu->key_down[i]) {
                cpu->V[cpu->key_wait_value] = i;
                cpu->key_waiting = 0;
                break;
            }
        }
    }else{// CPU 没在等按键，就可以执行下一条 CHIP-8 指令
      // 1. FETCH：从内存读 2 字节
       uint16_t op = (cpu->mem[cpu->PC] << 8) | cpu->mem[cpu->PC + 1];
       cpu->PC += 2;
       // 2. EXECUTE：解码并执行
       chip8_execute_opcode(cpu, op);
    }
#endif        
}

void chip8_execute_opcode(chip8_t *cpu, uint16_t op)
{
    switch (op & 0xF000) {
        case 0x0000:
            if (op == 0x00E0) op_00e0(cpu, op);// 清屏
            else if (op == 0x00EE) op_00ee(cpu, op);// 从子程序返回
            break;
        case 0x1000: op_1nnn(cpu, op); break;
        case 0x2000: op_2nnn(cpu, op); break;
        case 0x3000: op_3xkk(cpu, op); break;
        case 0x4000: op_4xkk(cpu, op); break;
        case 0x5000: op_5xy0(cpu, op); break;
        case 0x6000: op_6xkk(cpu, op); break;
        case 0x7000: op_7xkk(cpu, op); break;
        case 0x8000:
            switch (N(op)) {// 0x8XYN
                case 0x0: op_8xy0(cpu, op); break;// Vx = Vy
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
        case 0xA000: op_annn(cpu, op); break;// I = nnn
        case 0xB000: op_bnnn(cpu, op); break;// 跳转 V0 + nnn
        case 0xC000: op_cxkk(cpu, op); break;
        case 0xD000: op_dxyn(cpu, op); break;//画图
        case 0xE000:
            if ((op & 0x00FF) == 0x009E) op_ex9e(cpu, op);// 如果按下就跳一条指令
            else if ((op & 0x00FF) == 0x00A1) op_exa1(cpu, op);// 如果没按就跳一条指令
            break;
        case 0xF000:
            switch (op & 0x00FF) {// 0xFXNN
                case 0x07: op_fx07(cpu, op); break;// 将延迟定时器的值存入 Vx
                case 0x0A: op_fx0a(cpu, op); break;// 等待按键
                case 0x15: op_fx15(cpu, op); break;// 设置延迟定时器
                case 0x18: op_fx18(cpu, op); break;// 设置声音定时器
                case 0x1E: op_fx1e(cpu, op); break;// 将 Vx 的值加到 I 上
                case 0x29: op_fx29(cpu, op); break;// 将 I 设置为 Vx 字体的地址
                case 0x33: op_fx33(cpu, op); break; // 将 Vx 的值转换为 BCD 存入 I, I+1, I+2
                case 0x55: op_fx55(cpu, op); break;// 将 V0~Vx 的值存入 I 开始的内存
                case 0x65: op_fx65(cpu, op); break;// 将 I 开始的内存的值读入 V0~Vx
                default: break;
            }
            break;
        default:
            printf("Unknown opcode: 0x%04X\n", op);
            break;
    }
}

void chip8_disassemble(uint16_t op,
                       char *text,
                       size_t text_size)
{
    if (text == NULL || text_size == 0) {
        return;
    }

    uint8_t x = (op >> 8) & 0x0F;
    uint8_t y = (op >> 4) & 0x0F;
    uint8_t n = op & 0x0F;
    uint8_t kk = op & 0x00FF;
    uint16_t nnn = op & 0x0FFF;

    snprintf(text, text_size, "UNKNOWN");

    switch (op & 0xF000) {
    case 0x0000:
        if (op == 0x00E0) {
            snprintf(text, text_size, "CLS");
        } else if (op == 0x00EE) {
            snprintf(text, text_size, "RET");
        }
        break;

    case 0x1000:
        //snprintf(text, text_size, "JP 0x%03X", nnn);
        snprintf(text, text_size, "JP %03X", nnn);
        break;

    case 0x2000:
        //snprintf(text, text_size, "CALL 0x%03X", nnn);
        snprintf(text, text_size, "CALL %03X", nnn);
        break;

    case 0x3000:
        //snprintf(text, text_size, "SE V%X, 0x%02X", x, kk);
        snprintf(text, text_size, "SE V%X, %02X", x, kk);
        break;

    case 0x4000:
        //snprintf(text, text_size, "SNE V%X, 0x%02X", x, kk);
        snprintf(text, text_size, "SNE V%X, %02X", x, kk);
        break;

    case 0x5000:
        if (n == 0) {
            snprintf(text, text_size, "SE V%X, V%X", x, y);
        }
        break;

    case 0x6000:
        //snprintf(text, text_size, "LD V%X, 0x%02X", x, kk);
        snprintf(text, text_size, "LD V%X, %02X", x, kk);
        break;

    case 0x7000:
        //snprintf(text, text_size, "ADD V%X, 0x%02X", x, kk);
        snprintf(text, text_size, "ADD V%X, %02X", x, kk);
        break;

    case 0x8000:
        switch (n) {
        case 0x0:
            snprintf(text, text_size, "LD V%X, V%X", x, y);
            break;
        case 0x1:
            snprintf(text, text_size, "OR V%X, V%X", x, y);
            break;
        case 0x2:
            snprintf(text, text_size, "AND V%X, V%X", x, y);
            break;
        case 0x3:
            snprintf(text, text_size, "XOR V%X, V%X", x, y);
            break;
        case 0x4:
            snprintf(text, text_size, "ADD V%X, V%X", x, y);
            break;
        case 0x5:
            snprintf(text, text_size, "SUB V%X, V%X", x, y);
            break;
        case 0x6:
            snprintf(text, text_size, "SHR V%X", x);
            break;
        case 0x7:
            snprintf(text, text_size, "SUBN V%X, V%X", x, y);
            break;
        case 0xE:
            snprintf(text, text_size, "SHL V%X", x);
            break;
        default:
            break;
        }
        break;

    case 0x9000:
        if (n == 0) {
            snprintf(text, text_size, "SNE V%X, V%X", x, y);
        }
        break;

    case 0xA000:
        //snprintf(text, text_size, "LD I, 0x%03X", nnn);
        snprintf(text, text_size, "LD I, %03X", nnn);
        break;

    case 0xB000:
        //snprintf(text, text_size, "JP V0, 0x%03X", nnn);
        snprintf(text, text_size, "JP V0, %03X", nnn);
        break;

    case 0xC000:
        //snprintf(text, text_size, "RND V%X, 0x%02X", x, kk);
        snprintf(text, text_size, "RND V%X, %02X", x, kk);
        break;

    case 0xD000:
        snprintf(text, text_size, "DRW V%X, V%X, %X", x, y, n);
        break;

    case 0xE000:
        switch (kk) {
        case 0x9E:
            snprintf(text, text_size, "SKP V%X", x);
            break;
        case 0xA1:
            snprintf(text, text_size, "SKNP V%X", x);
            break;
        default:
            break;
        }
        break;

    case 0xF000:
        switch (kk) {
        case 0x07:
            snprintf(text, text_size, "LD V%X, DT", x);
            break;
        case 0x0A:
            snprintf(text, text_size, "LD V%X, K", x);
            break;
        case 0x15:
            snprintf(text, text_size, "LD DT, V%X", x);
            break;
        case 0x18:
            snprintf(text, text_size, "LD ST, V%X", x);
            break;
        case 0x1E:
            snprintf(text, text_size, "ADD I, V%X", x);
            break;
        case 0x29:
            snprintf(text, text_size, "LD F, V%X", x);
            break;
        case 0x33:
            snprintf(text, text_size, "LD B, V%X", x);
            break;
        case 0x55:
            snprintf(text, text_size, "LD [I], V%X", x);
            break;
        case 0x65:
            snprintf(text, text_size, "LD V%X, [I]", x);
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
}
