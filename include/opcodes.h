#ifndef OPCODES_H
#define OPCODES_H

#include "chip8.h"

/* ============================================================
   操作数提取宏
   ============================================================ */

#define NNN(op) ((op) & 0x0FFF)
#define KK(op)  ((op) & 0x00FF)
#define X(op)   (((op) & 0x0F00) >> 8)
#define Y(op)   (((op) & 0x00F0) >> 4)
#define N(op)   ((op) & 0x000F)

/* ============================================================
   系统类 (0x0000)
   ============================================================ */

void op_00e0(chip8_t *cpu, uint16_t op);   /* 清屏 */
void op_00ee(chip8_t *cpu, uint16_t op);   /* 返回 */

/* ============================================================
   控制流类
   ============================================================ */

void op_1nnn(chip8_t *cpu, uint16_t op);   /* 跳转 */
void op_2nnn(chip8_t *cpu, uint16_t op);   /* 调用子程序 */
void op_3xkk(chip8_t *cpu, uint16_t op);   /* Vx == kk 跳过 */
void op_4xkk(chip8_t *cpu, uint16_t op);   /* Vx != kk 跳过 */
void op_5xy0(chip8_t *cpu, uint16_t op);   /* Vx == Vy 跳过 */
void op_9xy0(chip8_t *cpu, uint16_t op);   /* Vx != Vy 跳过 */
void op_bnnn(chip8_t *cpu, uint16_t op);   /* 跳转 V0 + nnn */

/* ============================================================
   寄存器操作类
   ============================================================ */

void op_6xkk(chip8_t *cpu, uint16_t op);   /* Vx = kk */
void op_7xkk(chip8_t *cpu, uint16_t op);   /* Vx += kk */

/* ============================================================
   算术逻辑类 (0x8000)
   ============================================================ */

void op_8xy0(chip8_t *cpu, uint16_t op);   /* Vx = Vy */
void op_8xy1(chip8_t *cpu, uint16_t op);   /* Vx = Vx OR Vy */
void op_8xy2(chip8_t *cpu, uint16_t op);   /* Vx = Vx AND Vy */
void op_8xy3(chip8_t *cpu, uint16_t op);   /* Vx = Vx XOR Vy */
void op_8xy4(chip8_t *cpu, uint16_t op);   /* Vx = Vx + Vy (带进位) */
void op_8xy5(chip8_t *cpu, uint16_t op);   /* Vx = Vx - Vy (带借位) */
void op_8xy6(chip8_t *cpu, uint16_t op);   /* Vx = Vx >> 1 */
void op_8xy7(chip8_t *cpu, uint16_t op);   /* Vx = Vy - Vx */
void op_8xye(chip8_t *cpu, uint16_t op);   /* Vx = Vx << 1 */

/* ============================================================
   内存操作类
   ============================================================ */

void op_annn(chip8_t *cpu, uint16_t op);   /* I = nnn */

/* ============================================================
   随机数
   ============================================================ */

void op_cxkk(chip8_t *cpu, uint16_t op);   /* Vx = rand & kk */

/* ============================================================
   描画类
   ============================================================ */

void op_dxyn(chip8_t *cpu, uint16_t op);   /* 绘制精灵 */

/* ============================================================
   键盘类
   ============================================================ */

void op_ex9e(chip8_t *cpu, uint16_t op);   /* 按键按下跳过 */
void op_exa1(chip8_t *cpu, uint16_t op);   /* 按键未按跳过 */

/* ============================================================
   杂项类 (0xF000)
   ============================================================ */

void op_fx07(chip8_t *cpu, uint16_t op);   /* Vx = delay_timer */
void op_fx0a(chip8_t *cpu, uint16_t op);   /* 等待按键 */
void op_fx15(chip8_t *cpu, uint16_t op);   /* delay_timer = Vx */
void op_fx18(chip8_t *cpu, uint16_t op);   /* sound_timer = Vx */
void op_fx1e(chip8_t *cpu, uint16_t op);   /* I += Vx */
void op_fx29(chip8_t *cpu, uint16_t op);   /* I = font_addr */
void op_fx33(chip8_t *cpu, uint16_t op);   /* BCD转换 */
void op_fx55(chip8_t *cpu, uint16_t op);   /* 存储寄存器到内存 */
void op_fx65(chip8_t *cpu, uint16_t op);   /* 从内存加载寄存器 */

#endif