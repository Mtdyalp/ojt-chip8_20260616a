# Phase 2 课题：CHIP-8 模拟器

**周期：** 3 周
**目的：** 掌握 CPU 模拟（fetch/decode/execute）、内存管理、外设控制。这是嵌入式软件开发的核心技能。

---

## 项目概要

CHIP-8 是 1970 年代设计的一台虚拟机，具有以下特征：

- 4KB 内存
- 16 个 8 位通用寄存器
- 64×32 像素的单色屏幕
- 16 键输入手柄
- 35 条指令

它是最简单的模拟器，且教材丰富，是模拟器开发入门的最佳选择。
用 SDL2 实现画面输出、按键输入、蜂鸣声，并运行实际的游戏 ROM（如 Pong 等）。

---

## CHIP-8 架构

### 内存映射

```
+------------------+ 0xFFF (4095)
|                  |
|     程序区域      |
|   (ROM 加载到    |
|     这里)        |
|                  |
+------------------+ 0x200 (512)
|    系统保留区     |
|   (含字库数据    |
|     等)          |
+------------------+ 0x000
```

- 整体：4096 字节（0x000〜0xFFF）
- ROM 从 0x200 开始放置
- 0x000〜0x1FF 为系统保留区（含内置字库数据）

### 寄存器

| 寄存器 | 大小 | 用途 |
|----------|--------|------|
| V0〜VF | 8 位 ×16 | 通用寄存器。VF 同时用作标志寄存器 |
| I | 16 位 | 指定内存地址用的索引寄存器 |
| PC | 16 位 | 程序计数器（下一条要执行的指令的地址） |
| SP | 8 位 | 栈指针 |
| DT | 8 位 | 延迟定时器（以 60Hz 递减，倒数到 0 为止） |
| ST | 8 位 | 声音定时器（以 60Hz 递减，大于 0 期间发出蜂鸣声） |

### 栈

- 16 层
- 子程序调用（CALL）时保存返回地址

### 屏幕

- 分辨率：64×32 像素（单色）
- 精灵以 XOR 方式描画（与已有像素重叠则消除 → VF 置为碰撞标志）
- 内置字库：0〜F 共 16 个字符（每个 5 字节，4×5 像素）

### 键盘

将 16 键（0x0〜0xF）按如下方式映射到键盘：

```
CHIP-8 键盘            键盘映射
┌───┬───┬───┬───┐      ┌───┬───┬───┬───┐
│ 1 │ 2 │ 3 │ C │      │ 1 │ 2 │ 3 │ 4 │
├───┼───┼───┼───┤      ├───┼───┼───┼───┤
│ 4 │ 5 │ 6 │ D │  →   │ Q │ W │ E │ R │
├───┼───┼───┼───┤      ├───┼───┼───┼───┤
│ 7 │ 8 │ 9 │ E │      │ A │ S │ D │ F │
├───┼───┼───┼───┤      ├───┼───┼───┼───┤
│ A │ 0 │ B │ F │      │ Z │ X │ C │ V │
└───┴───┴───┴───┘      └───┴───┴───┴───┘
```

### 指令集（35 条）

全部指令均为 2 字节（16 位）。使用以下记号：
- `nnn`：12 位地址
- `nn` / `kk`：8 位常量
- `x`, `y`：4 位寄存器编号（V0〜VF）
- `n`：4 位常量

#### 控制流

| Opcode | 助记符 | 动作 |
|--------|-------------|------|
| 00E0 | CLS | 清屏 |
| 00EE | RET | 从子程序返回 |
| 1nnn | JP addr | PC = nnn |
| 2nnn | CALL addr | 调用子程序（将当前 PC 压栈） |
| Bnnn | JP V0, addr | PC = nnn + V0 |

#### 条件分支（跳过）

| Opcode | 助记符 | 动作 |
|--------|-------------|------|
| 3xkk | SE Vx, byte | 若 Vx == kk 则跳过下一条指令 |
| 4xkk | SNE Vx, byte | 若 Vx != kk 则跳过下一条指令 |
| 5xy0 | SE Vx, Vy | 若 Vx == Vy 则跳过下一条指令 |
| 9xy0 | SNE Vx, Vy | 若 Vx != Vy 则跳过下一条指令 |

#### 寄存器操作

| Opcode | 助记符 | 动作 |
|--------|-------------|------|
| 6xkk | LD Vx, byte | Vx = kk |
| 7xkk | ADD Vx, byte | Vx = Vx + kk（无进位标志） |
| 8xy0 | LD Vx, Vy | Vx = Vy |
| 8xy1 | OR Vx, Vy | Vx = Vx OR Vy |
| 8xy2 | AND Vx, Vy | Vx = Vx AND Vy |
| 8xy3 | XOR Vx, Vy | Vx = Vx XOR Vy |
| 8xy4 | ADD Vx, Vy | Vx = Vx + Vy，溢出时 VF = 1 |
| 8xy5 | SUB Vx, Vy | Vx = Vx - Vy，无借位时 VF = 1 |
| 8xy6 | SHR Vx | Vx 右移 1 位，移出的位存入 VF |
| 8xy7 | SUBN Vx, Vy | Vx = Vy - Vx，无借位时 VF = 1 |
| 8xyE | SHL Vx | Vx 左移 1 位，移出的位存入 VF |

#### 内存・特殊

| Opcode | 助记符 | 动作 |
|--------|-------------|------|
| Annn | LD I, addr | I = nnn |
| Cxkk | RND Vx, byte | Vx = (随机数) AND kk |
| Dxyn | DRW Vx, Vy, n | 在坐标 (Vx, Vy) 描画 n 字节的精灵。发生碰撞时 VF = 1 |

#### 输入

| Opcode | 助记符 | 动作 |
|--------|-------------|------|
| Ex9E | SKP Vx | 若按下了键 Vx 则跳过 |
| ExA1 | SKNP Vx | 若未按下键 Vx 则跳过 |
| Fx0A | LD Vx, K | 等待按键输入（阻塞），将按下的键存入 Vx |

#### 定时器・工具类

| Opcode | 助记符 | 动作 |
|--------|-------------|------|
| Fx07 | LD Vx, DT | Vx = DT |
| Fx15 | LD DT, Vx | DT = Vx |
| Fx18 | LD ST, Vx | ST = Vx |
| Fx1E | ADD I, Vx | I = I + Vx |
| Fx29 | LD F, Vx | I = 与 Vx 的值对应的字库精灵的地址 |
| Fx33 | LD B, Vx | 将 Vx 的值转换为 BCD（百位・十位・个位），存入 I, I+1, I+2 |
| Fx55 | LD [I], Vx | 将 V0〜Vx 存入从 I 号地址起的内存 |
| Fx65 | LD Vx, [I] | 从 I 号地址起的内存读入 V0〜Vx |

---

## 功能规格

### 第 1 周：理解规格 + 设计数据结构 + 开始实现指令

- 阅读 CHIP-8 规格书，理解架构
- 设计表示 CPU 状态的 C 结构体（寄存器・内存・栈・画面缓冲）
- 内存初始化（放置内置字库数据）
- 将 ROM 文件加载到 0x200 的处理
- 实现 fetch/decode/execute 循环的骨架
- 从寄存器操作类指令（6xkk, 7xkk, 8xy0〜8xyE）开始实现

**第 1 周完成条件：**
- [ ] 已定义 CHIP-8 的结构体
- [ ] 内置字库已放入内存
- [ ] 能加载 ROM 文件
- [ ] fetch/decode/execute 循环可运行
- [ ] 寄存器操作类的指令测试通过

### 第 2 周：完成指令实现 + 集成 SDL2

- 实现剩余的全部指令（控制流・描画・输入・定时器类）
- 用 SDL2 描画 64×32 的画面（像素放大显示）
- 将键盘输入映射到 CHIP-8 键盘
- 以 60Hz 更新延迟定时器・声音定时器
- 声音定时器 > 0 时发出蜂鸣声
- 用测试 ROM（如 IBM Logo 等）确认基本动作

**第 2 周完成条件：**
- [ ] 35 条指令全部实现
- [ ] `make test` 指令测试通过
- [ ] 画面可显示，精灵可描画
- [ ] 键盘输入可工作
- [ ] 测试 ROM 正确显示

### 第 3 周：调试・完成・演示

- 调试到 Pong ROM 可以对战游玩为止
- 用另一个游戏 ROM（Tetris、Space Invaders 等）也确认动作
- 用 Valgrind 修复全部内存泄漏
- 整理代码
- 向上级演示

**第 3 周完成条件：**
- [ ] Pong 可以玩
- [ ] 另一个游戏 ROM 可运行
- [ ] `make test` 全部测试 OK
- [ ] Valgrind 泄漏 0 件
- [ ] 向上级演示完成

---

## 文件构成

```
ojt-chip8/
├── Makefile
├── include/
│   ├── chip8.h        # CHIP-8 状态结构体（寄存器・内存・栈・画面）
│   ├── opcodes.h      # 指令执行函数
│   ├── display.h      # SDL2 描画
│   ├── input.h        # 键盘输入
│   └── audio.h        # 蜂鸣声
├── src/
│   ├── main.c         # ROM 读取・模拟循环
│   ├── chip8.c        # 初始化・fetch/decode/execute
│   ├── opcodes.c      # 35 条指令的实现
│   ├── display.c      # SDL2 画面描画
│   ├── input.c        # 按键输入处理
│   └── audio.c        # 由声音定时器驱动的蜂鸣
├── roms/              # 测试 ROM（Pong, Tetris 等）
├── test/
│   └── test_cpu.c     # 指令单元测试（不依赖 SDL2）
└── README.md
```

---

## 技术要求

### Makefile

- `make` / `make clean` / `make debug` / `make test`
- `make test` 在不依赖 SDL2 的情况下执行指令测试
- 链接标志：`-lSDL2 -lSDL2_mixer`（SDL2 已在上一个项目中引入）

### 运行方式

```bash
./chip8 roms/pong.ch8
```

通过命令行参数指定 ROM 文件启动。

### 测试 ROM 的获取

使用互联网上公开的 CHIP-8 测试 ROM。自行查找并放到 `roms/` 下。
至少准备以下几个：

- **IBM Logo**：画面描画的基本测试（Logo 正确显示则说明 DRW 指令在工作）
- **Pong**：作为游戏的综合测试
- 另外任选 1 个游戏 ROM

### Valgrind

- 退出后应当泄漏 0 件
- SDL2 内部的内存不在抑制范围内

### Git 管理

- 用本仓库（Bitbucket：`vividnode/ojt-chip8`）管理
- 提交要细分（最少 20 commits）
- 提交信息用英文

### AI 使用规则

**本项目禁止用 AI 生成代码。**

| 允许 | 禁止 |
|------|------|
| 就 CHIP-8 的指令规格向 AI 提问 | 让 AI「写一个 CHIP-8 模拟器」 |
| 就 opcode 的位操作方法向 AI 提问 | 让 AI 写代码 |
| 就调试方针向 AI 咨询 | 把代码贴给 AI 让它「改」 |

### 日报要求

每天的日报里不仅要写「做了什么」，还要写 **「为什么这么做」**。

不好的例子：
```
- 实现了 8xy4 指令
```

好的例子：
```
- 实现了 8xy4（ADD Vx, Vy）。当 Vx + Vy 超过 255 时需要置 VF = 1。
  最初我把 VF 的设置放在加法之后做，但当 Vx == VF 时会被覆盖，
  所以改成先做进位判断再做加法。
```

---

## 测试用例（必须）

把指令的实现部分分离成不依赖 SDL2 的形式，用 `test/test_cpu.c` 自动验证。

```bash
$ make test
$ ./test_cpu
Test 01 (LD Vx, byte):         [OK]
Test 02 (ADD Vx, byte):        [OK]
Test 03 (LD Vx, Vy):           [OK]
Test 04 (OR/AND/XOR):          [OK]
Test 05 (ADD Vx, Vy + carry):  [OK]
Test 06 (SUB + borrow):        [OK]
Test 07 (SHR/SHL):             [OK]
Test 08 (JP/CALL/RET):         [OK]
Test 09 (SE/SNE skip):         [OK]
Test 10 (LD I / ADD I):        [OK]
Test 11 (BCD):                 [OK]
Test 12 (LD [I] / LD Vx [I]):  [OK]
Test 13 (DRW collision):       [OK]
Test 14 (font loading):        [OK]
Test 15 (ROM loading):         [OK]

All 15 tests passed.
```

**Valgrind 测试：**
```bash
$ valgrind --leak-check=full ./test_cpu 2>&1 | grep -E "(All heap|LEAK SUMMARY|definitely)" -A 2
# 期望: All heap blocks were freed -- no leaks are possible
```

---

## 验收检查清单

演示时由上级实际操作确认。

### 基本动作

- [ ] `make` 能构建（0 警告）
- [ ] `./chip8 roms/xxxx.ch8` 能启动
- [ ] 不指定 ROM 启动时，显示用法
- [ ] IBM Logo ROM 正确显示

### 游戏动作

- [ ] Pong 可运行（画面显示・球拍操作・小球反弹・显示得分）
- [ ] 另一个游戏 ROM 可运行
- [ ] 按键映射正确
- [ ] 定时器在工作（游戏速度合适）
- [ ] 有蜂鸣声

### 质量

- [ ] `make test` 全部测试 OK
- [ ] Valgrind 泄漏 0 件（自己代码导致的）
- [ ] Bitbucket 上有 20 commits 以上

### 现场变更要求（任选 1 项实施）

- 改一下画面的放大倍率（例：10 倍 → 15 倍）
- 改一下像素颜色（黑白 → 绿黑）
- 改一下模拟速度（变快 / 变慢）
- 改一个按键映射

---

## 追加课题：调试器功能（1 周）

在 CHIP-8 模拟器的基本动作完成之后再着手。

**日程目安（参考）：** 推进方式自由。以下只是一例。

| 日 | 内容示例 |
|----|--------|
| Day 1 | 状态机（RUNNING/PAUSED/STEPPING）+ Space/N 键控制 |
| Day 2 | 扩展窗口 + 用 SDL2_ttf 显示寄存器面板 |
| Day 3 | 反汇编器（opcode → 助记符字符串转换，显示 PC 前后的指令） |
| Day 4 | 断点管理 + CLI 参数解析（`-d`, `-b`）+ 栈显示 |
| Day 5 | 调试・整理・演示 |

### 概要

给模拟器内嵌一个**交互式调试器**。
实际的嵌入式开发中会用 GDB 或探针在目标机上调试程序。
本课题让你体验的不是「使用调试器的一方」，而是「制作调试器的一方」，
理解 CPU 的执行控制・状态可视化是如何实现的。

### 画面布局

调试模式时把窗口横向加宽，在右侧显示调试面板。

```
┌─────── CHIP-8 画面 (64x32) ───────┬──────── 调试面板 ────────┐
│                                    │ [PAUSED]  Step: N  Resume: Space │
│                                    │                                  │
│         (游戏画面)                  │ == Registers ==                  │
│                                    │ V0:0A V1:02 V2:00 V3:FF         │
│                                    │ V4:00 V5:00 V6:1A V7:00         │
│                                    │ V8:00 V9:00 VA:02 VB:00         │
│                                    │ VC:00 VD:00 VE:00 VF:01         │
│                                    │ I:0300  PC:0210  SP:02           │
│                                    │ DT:1E  ST:00                     │
│                                    │                                  │
│                                    │ == Disassembly ==                │
│                                    │   0x020C: 6A02  LD VA, 02        │
│                                    │   0x020E: 6B0C  LD VB, 0C        │
│                                    │ > 0x0210: D015  DRW V0, V1, 5    │
│                                    │   0x0212: 7001  ADD V0, 01        │
│                                    │   0x0214: 3040  SE V0, 40        │
│                                    │                                  │
│                                    │ == Stack ==                      │
│                                    │ [0] 0x0200                       │
│                                    │ [1] 0x0248                       │
│                                    │                                  │
│                                    │ == Breakpoints ==                │
│                                    │ * 0x0220                         │
│                                    │ * 0x0300                         │
└────────────────────────────────────┴──────────────────────────────────┘
```

- `>` 表示当前 PC 位置
- `*` 表示设置了断点的地址
- 通常执行中可以不显示调试面板的文字（为了性能）
- 调试面板的文字描画使用 SDL2_ttf（在 SDL2 Snake 中已用过）
- 不需要额外的 UI 框架。全部仅用 SDL2 实现

### 功能要求

**执行控制：**
- 通常执行 / 暂停的切换（`Space` 键）
- 暂停中单步执行（`N` 键：只执行 1 条指令后再次暂停）
- 设置断点（到达指定地址时自动暂停）

**状态显示（暂停中显示在画面上）：**
- 全部寄存器（V0〜VF, I, PC, SP, DT, ST）
- 栈的内容
- 反汇编显示 PC 附近的内存（前后数条指令）
- 当前键盘状态

**断点管理：**
- 启动时用命令行参数或文件指定断点
- 或在执行中用按键操作给当前 PC 添加/删除断点

### 启动方式

```bash
# 通常执行
./chip8 roms/pong.ch8

# 以调试模式启动（从暂停状态开始）
./chip8 -d roms/pong.ch8

# 带断点启动
./chip8 -d -b 0x200 -b 0x220 roms/pong.ch8
```

### 验收检查清单

- [ ] `-d` 选项以暂停状态开始
- [ ] `Space` 能切换执行/暂停
- [ ] 暂停中能在画面上显示全部寄存器的值
- [ ] `N` 能逐条执行指令，能确认寄存器的变化
- [ ] PC 附近的指令被反汇编显示（例：`0x200: 6A02  LD VA, 02`）
- [ ] 到达断点时自动暂停
- [ ] 能把 Pong 中途停下，确认寄存器状态，并单步逐条往前推进

### 现场变更要求（任选 1 项实施）

- 改一下断点地址，在别的地方停下试试
- 添加一个显示指定内存地址内容的功能
- 添加改变执行速度的键（`+` / `-`）

---

## 学习参考资料

### CHIP-8 规格

| 资料 | 说明 |
|------|------|
| [Guide to making a CHIP-8 emulator（Tobias V. Langhoff）](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/) | 最易懂的讲解。每条指令的动作都写得很细 |
| [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) | 经典的技术参考。全部指令的详细规格 |
| [CHIP-8 — Wikipedia](https://en.wikipedia.org/wiki/CHIP-8) | 架构概要 |

### 测试 ROM

| 资料 | 说明 |
|------|------|
| [Timendus/chip8-test-suite](https://github.com/Timendus/chip8-test-suite) | 能自动测试指令正确性的 ROM 集 |
| [dmatlack/chip8 — games](https://github.com/dmatlack/chip8/tree/master/roms) | Pong, Tetris 等经典游戏 ROM |

### 模拟器开发的参考

| 资料 | 说明 |
|------|------|
| [How to write an emulator (CHIP-8 interpreter)（Laurence Muller）](https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/) | C 语言的实现讲解（可参考，但不要复制代码） |
