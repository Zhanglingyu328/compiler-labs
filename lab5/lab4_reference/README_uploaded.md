# SLR(1) 分析表生成实验

这个项目按实验要求分成两部分：

1. **必做部分**：手写 SLR(1) 分析表生成器。
2. **选做部分**：使用 Flex / Bison 体验自动化词法、语法分析工具。

UI 页面暂时不包含在本 ZIP 中。当前项目已经导出 `slr1_result.json`、`action.csv`、`goto.csv`，后续可以直接接入你已有的 UI 页面。

---

## 一、项目结构

```txt
slr1_experiment/
├── Makefile
├── README.md
├── src/
│   └── main.cpp
├── examples/
│   ├── grammar_expr.txt
│   └── grammar_if_else_conflict.txt
├── optional/
│   └── flex_bison/
│       ├── expr.l
│       ├── expr.y
│       └── Makefile
└── out/
```

---

## 二、必做部分功能

`src/main.cpp` 已经实现：

- 读取文法文件
- 自动增广文法
- 计算 FIRST 集
- 计算 FOLLOW 集
- 构造 LR(0) 项目集族
- 生成 SLR(1) ACTION 表
- 生成 SLR(1) GOTO 表
- 检测移进-归约、归约-归约冲突
- 输出分析过程
- 导出：
  - `out/report.txt`
  - `out/action.csv`
  - `out/goto.csv`
  - `out/slr1_result.json`

---

## 三、编译运行

进入项目目录：

```bash
cd slr1_experiment
```

编译：

```bash
make
```

运行默认表达式文法：

```bash
make run
```

带输入串分析：

```bash
make parse
```

等价于：

```bash
./slr1 --grammar examples/grammar_expr.txt --out-dir out --parse "id + id * id"
```

---

## 四、文法文件格式

示例：

```txt
%start E

E -> E + T | T
T -> T * F | F
F -> ( E ) | id
```

注意：

- 每个符号之间要用空格分开。
- `|` 表示候选式。
- `%start` 指定开始符号。
- 空串可以写成 `ε`、`epsilon`、`eps` 或 `@`。
- 输入串也要用空格分开，例如：`id + id * id`。

---

## 五、实验报告可写的核心说明

SLR(1) 分析表基于 LR(0) 项目集族构造，但归约动作不再像 LR(0) 那样在所有终结符上填写，而是只在产生式左部非终结符的 FOLLOW 集上填写。

例如项目：

```txt
E -> T .
```

如果产生式编号是 `2. E -> T`，那么只在 `FOLLOW(E)` 中的终结符上填写 `r2`。

这样可以减少 LR(0) 中的部分移进-归约冲突。

---

## 六、选做部分：Flex / Bison

进入选做目录：

```bash
cd optional/flex_bison
```

编译：

```bash
make
```

运行：

```bash
make run
```

或者手动输入：

```bash
./expr_parser
```

然后输入：

```txt
id + id * id
```

说明：

- `expr.l` 是 Flex 词法分析文件。
- `expr.y` 是 Bison 语法分析文件。
- 这部分用于和手写 SLR(1) 生成器进行对比。

如果你的环境没有安装工具，可以在 Ubuntu / Debian 中安装：

```bash
sudo apt update
sudo apt install flex bison
```

---

## 七、可继续拓展方向

当前代码已经预留了适合接 UI 的输出文件。后续可以继续做：

1. **UI 接入**：读取 `out/slr1_result.json`，渲染项目集、ACTION 表、GOTO 表。
2. **文法编辑器**：在页面输入文法，然后调用后端生成表。
3. **分析动画**：把 `--parse` 的分析过程改成 JSON，前端逐步展示状态栈、符号栈、输入串、动作。
4. **冲突可视化**：把冲突单元格标红，点击显示冲突来源项目。
5. **更多文法支持**：支持 quoted token、多字符终结符、多个开始符号选择。
6. **导出功能**：导出 Markdown、HTML、Excel 或图片。

---

## 八、提交建议

实验提交时建议展示：

1. 文法产生式编号。
2. FIRST / FOLLOW 集。
3. LR(0) 项目集族。
4. ACTION 表。
5. GOTO 表。
6. 输入串 `id + id * id` 的分析过程。
7. 选做 Flex / Bison 运行截图。
8. 后续 UI 拓展说明。
