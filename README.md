# 编译原理实验合集 Lab1-Lab4

本仓库整理了编译原理课程前四次实验的代码、测试文件、运行结果与综合可视化网页。四次实验从 DFA 字符串识别开始，逐步扩展到类 C 语言词法分析、LR(0) 项目集规范族构造，最后完成 SLR(1) 分析表生成，并通过一个 HTML 页面把实验 1-4 的核心过程统一展示出来。

## 项目概述

| 实验 | 主题 | 核心目标 | 完成情况 |
|---|---|---|---|
| Lab1 | DFA 模拟程序设计 | 读取 DFA 五元组，检查 DFA 合法性，枚举合法字符串，模拟字符串识别过程 | 已完成必做、选做和 DFA 可视化页面 |
| Lab2 | 类 C 语言词法分析器 Scanner | 使用 DFA/手写扫描器识别类 C 语言 token，输出 token 序列 | 已完成必做、Flex 选做和 Scanner 可视化页面 |
| Lab3 | LR(0) 项目集规范族构造 | 输入文法，增广文法，计算 Closure/Goto，生成 LR(0) 项目集规范族 | 已完成必做、冲突检测、状态图和 Web UI |
| Lab4 | SLR(1) 分析表生成 | 基于 LR(0) 规范族和 FOLLOW 集生成 ACTION/GOTO 表，并完成输入串分析 | 已完成必做、Flex/Bison 选做和 1-4 综合 UI 页面 |

## 实验环境

主要开发和运行环境如下：

```text
操作系统：openEuler / Linux 云端环境
运行平台：鲲鹏服务器 / ARM64
开发方式：VS Code + SSH 远程连接 / 服务器终端
主要语言：C / C++ / HTML / CSS / JavaScript
编译工具：gcc / g++ / make
选做工具：flex / bison
```

## 推荐目录结构

```text
compiler-labs/
├── README.md
├── lab1/
│   ├── main.cpp
│   ├── dfa_basic.txt
│   ├── dfa_to.txt
│   └── ui/
├── lab2/
│   ├── scanner.cpp
│   ├── input.txt
│   ├── input2.txt
│   ├── test.c
│   ├── token_output.txt
│   ├── flex_scanner.l
│   └── ui/
├── lab3/
│   ├── Makefile
│   ├── examples/
│   │   ├── grammar.txt
│   │   └── test.src
│   ├── src/
│   └── out/
├── lab4/
│   ├── Makefile
│   ├── examples/
│   │   └── grammar_expr.txt
│   ├── src/
│   │   └── main.cpp
│   ├── out/
│   │   ├── report.txt
│   │   ├── action.csv
│   │   ├── goto.csv
│   │   └── slr1_result.json
│   ├── optional/
│   │   └── flex_bison/
│   └── ui/
│       └── compiler_labs_1_2_3_4_integrated.html
└── docs/
```

实际目录可以根据自己的服务器目录调整，只要保证每个实验的源码、输入文件和运行说明完整即可。

---

# Lab1：DFA 模拟程序设计

## 实验要求

实验一要求编写 DFA 模拟程序，能够从文本文件读取 DFA 五元组，包括：

- 字符集；
- 状态集；
- 开始状态；
- 接受状态集；
- 状态转移函数。

程序还需要完成 DFA 合法性检查，并实现两个核心功能：

1. 输入最大长度 `N`，输出该 DFA 语言中长度不超过 `N` 的所有合法字符串；
2. 输入一个字符串，模拟 DFA 的识别过程，输出完整状态转移路径，并判断该字符串是否属于该语言。

## 我完成的内容

### 1. 必做部分

我实现了一个 C++ 版本的 DFA 模拟程序，使用文本文件保存 DFA 五元组。

必做案例为：识别所有 **以 `ab` 结尾** 的字符串。

对应语言为：

```text
L = { w | w 以 ab 结尾 }
```

DFA 状态设计如下：

```text
q0：初始状态，尚未形成后缀 ab
q1：已经读到字符 a
q2：已经形成后缀 ab，接受状态
```

输入文件为 `dfa_basic.txt`，主要内容包括：

```text
alphabet: a b
states: q0 q1 q2
start: q0
accept: q2
transitions:
q0 a q1
q0 b q0
q1 a q1
q1 b q2
q2 a q1
q2 b q0
```

实现功能：

- 读取 DFA 文件；
- 输出 DFA 基本信息；
- 检查开始状态是否合法；
- 检查接受状态是否为空、是否属于状态集；
- 检查转移函数是否完整；
- 枚举长度不超过 `N` 的合法字符串；
- 对输入串进行逐步识别；
- 对非法字符串、非法字符、错误 DFA 文件进行测试。

### 2. 选做部分

选做部分我结合教材示例，设计了一个识别 **包含子串 `to`** 的 DFA。

对应语言为：

```text
L = { w | w 中含有子串 to }
```

状态设计如下：

```text
q0：初始状态，尚未读到 t
q1：已经读到 t
q2：已经匹配到子串 to，接受状态
```

输入文件为 `dfa_to.txt`，程序可以读取该文件，并判断输入字符串是否包含子串 `to`。

### 3. Lab1 网页实现

我为实验一实现了 DFA 可视化页面，主要功能包括：

- 必做案例和选做案例切换；
- DFA 五元组展示；
- 状态转移表展示；
- 输入字符串判定；
- 长度不超过 `N` 的合法字符串枚举；
- 自定义 DFA 输入；
- 模拟终端输出运行过程。

---

# Lab2：类 C 语言词法分析器 Scanner

## 实验要求

实验二要求实现一个类 C 语言词法分析器 scanner。词法分析器需要把源程序中的字符序列转换为 token 序列，为后续语法分析提供输入。

实验要求包括：

1. 对若干个独立符号串进行分类；
2. 对一整行类 C 代码进行扫描；
3. 能够识别主要词法单位，例如标识符、整数、实数、关键字、算术运算符、关系运算符、赋值符号、括号和分号等；
4. 能够将 token 序列输出到文件，作为后续语法分析输入。

## 我完成的内容

### 1. 必做部分

我实现了一个手写 C++ scanner，支持三种运行模式：

```text
模式 1：对多个独立字符串进行分类
模式 2：对一行类 C 代码进行扫描
模式 3：读取 test.c 文件并输出 token_output.txt
```

识别的 token 类型包括：

| 词法单元 | 示例 | 输出类型 |
|---|---|---|
| 标识符 | `a`、`qsort`、`low` | `ID` |
| 整数 | `0`、`485`、`123` | `INT` |
| 实数 | `3.14`、`841.6541` | `FLO` |
| 关键字 | `int`、`float`、`void`、`while`、`return` | `INT`、`FLOAT`、`VOID`、`WHILE`、`RETURN` |
| 算术运算符 | `+`、`-`、`*`、`/` | `ADD`、`MUL` |
| 关系运算符 | `<`、`>`、`<=`、`>=`、`==`、`!=` | `ROP` |
| 赋值符号 | `=` | `ASG` |
| 括号和界符 | `()`、`[]`、`{}`、`,`、`;` | `LPA`、`RPA`、`LBK`、`RBK`、`LBR`、`RBR`、`CMA`、`SCO` |
| 自增自减 | `++`、`--` | `AAA`、`AAS` |

我还完成了以下增强功能：

- 支持 `//` 单行注释跳过；
- 支持词法错误定位；
- 遇到非法字符后不直接退出，而是记录错误并继续分析；
- 将 `test.c` 扫描结果写入 `token_output.txt`；
- token 输出可以作为后续语法分析实验的输入。

### 2. 选做部分

选做部分我使用 `flex` 实现了基于正则式的词法分析器。

主要正则式包括：

```text
DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INTNUM      [+-]?{DIGIT}+
FLOATNUM    [+-]?{DIGIT}*"."{DIGIT}+
```

完成内容：

- 编写 `flex_scanner.l`；
- 使用 `flex` 生成 `lex.yy.c`；
- 使用 `gcc` 编译生成可执行程序；
- 使用单行代码测试；
- 使用 `test.c` 测试；
- 将输出保存到 `flex_output.txt`；
- 对比 flex 版本和手写 scanner 版本的输出结果。

### 3. Lab2 网页实现

在网页中，我将实验一 DFA 模块和实验二 scanner 模块整合到一个页面中。

Scanner 页面功能包括：

- 输入类 C 源代码；
- 点击按钮执行词法分析；
- 表格输出 token 类型、属性值、行号和列号；
- 显示词法错误；
- 统计各类 token 数量；
- 加载示例代码；
- 清空分析结果。

该页面体现了 DFA 与 scanner 的关系：正则式描述词法规则，DFA 识别字符串，scanner 则把多个 token 的识别过程组合起来处理源程序。

---

# Lab3：LR(0) 项目集规范族构造

## 实验要求

实验三要求构造给定文法的 LR(0) 项目集规范族，为后续生成 SLR(1) 分析表提供基础。

主要要求包括：

1. 输入文法规则；
2. 支持扩展巴科斯范式；
3. 增广文法；
4. 从初始项目开始构造 I0；
5. 计算 Closure 闭包；
6. 计算 Goto 状态转移；
7. 重复构造新的项目集，直到不再产生新状态；
8. 输出所有 LR(0) 项目集；
9. 输出状态转移关系；
10. 检测移进-归约冲突或归约-归约冲突。

## 我完成的内容

### 1. 必做部分

我实现了 LR(0) 项目集规范族生成程序。

输入文法示例：

```text
E -> E + T | T
T -> T * F | F
F -> ( E ) | id | num
```

程序自动增广文法：

```text
E' -> E
```

并生成完整产生式列表：

```text
0. E' -> E
1. E  -> E + T
2. E  -> T
3. T  -> T * F
4. T  -> F
5. F  -> ( E )
6. F  -> id
7. F  -> num
```

程序实现功能：

- 文法读取；
- 自动增广文法；
- Closure 闭包计算；
- Goto 转移计算；
- LR(0) 项目集规范族生成；
- 内核项展示；
- 状态转移关系输出；
- LR(0) 冲突检测；
- 输出 JSON 文件；
- 输出 Graphviz `.dot` 文件；
- 生成状态图。

实验结果中，表达式文法生成了：

```text
productions: 8
states: 13
transitions: 26
conflicts: 2
```

冲突出现在：

```text
I2：Shift-Reduce Conflict
I10：Shift-Reduce Conflict
```

原因是这些状态中同时存在归约项目和移进项目，说明该表达式文法不是严格 LR(0) 文法，但可以继续作为 SLR(1) 分析表构造的基础。

### 2. 选做部分

选做部分我完成了实验 1、实验 2、实验 3 的 UI 串联。

主要包括：

- DFA 可视化实验；
- 类 C 词法分析器；
- LR(0) 项目集规范族构造；
- 实验二 token 输出和实验三语法分析输入的联系展示；
- 项目集卡片展示；
- Goto 表展示；
- 状态图展示；
- JSON 输出展示。

### 3. Lab3 网页实现

LR(0) 页面可以直接输入文法，点击按钮后自动展示：

- 增广文法；
- 产生式数量；
- 状态数量；
- 转移数量；
- 冲突数量；
- 所有项目集；
- Kernel Items；
- Closure Items；
- Goto 转移；
- Goto 表；
- 状态图；
- JSON 数据。

该页面让命令行中的 Closure、Goto 和项目集构造过程变得更加直观。

---

# Lab4：SLR(1) 分析表生成

## 实验要求

实验四要求在实验三 LR(0) 项目集规范族的基础上生成 SLR(1) 分析表。

输入包括：

```text
实验三生成的 LR(0) 规范族
文法符号的 FOLLOW 集
```

输出为：

```text
SLR(1) 分析表，即 ACTION / GOTO 二维表
```

SLR(1) 与 LR(0) 的主要区别是：SLR(1) 在归约时引入 FOLLOW 集，只在当前输入符号属于产生式左部非终结符的 FOLLOW 集时才进行归约，从而减少 LR(0) 中的部分冲突。

## 我完成的内容

### 1. 必做部分

我实现了 C++17 版本的 SLR(1) 分析表生成程序。

实验使用表达式文法：

```text
E -> E + T
E -> T
T -> T * F
T -> F
F -> ( E )
F -> id
```

程序自动增广为：

```text
0. E' -> E
1. E  -> E + T
2. E  -> T
3. T  -> T * F
4. T  -> F
5. F  -> ( E )
6. F  -> id
```

实现功能包括：

- 读取文法文件；
- 自动增广文法；
- 计算 FIRST 集；
- 计算 FOLLOW 集；
- 构造 LR(0) 项目集规范族；
- 输出 GOTO 转换关系；
- 生成 SLR(1) ACTION 表；
- 生成 SLR(1) GOTO 表；
- 检测移进-归约冲突和归约-归约冲突；
- 对输入串进行 SLR(1) 分析；
- 输出分析过程；
- 导出 `report.txt`、`action.csv`、`goto.csv`、`slr1_result.json`。

必做程序运行后，表达式文法无冲突：

```text
无冲突：该文法在当前构造下是 SLR(1) 文法。
```

输入串测试：

```text
id + id * id
```

最终分析到：

```text
acc，分析成功
```

说明 SLR(1) 分析表生成和输入串分析过程都正确。

### 2. 选做部分

选做部分我完成了 Flex/Bison 表达式语法分析器。

目录：

```text
lab4/optional/flex_bison/
```

主要文件：

```text
expr.l
expr.y
Makefile
```

完成内容：

- 使用 `flex` 编写词法规则；
- 使用 `bison` 编写表达式语法规则；
- 生成 `expr.tab.c`、`expr.tab.h`、`lex.yy.c`；
- 编译生成 `expr_parser`；
- 测试表达式 `id + id * id`；
- 输出 `语法分析成功`。

运行过程中，鲲鹏服务器缺少 `libfl`，编译时报错：

```text
/usr/bin/ld: cannot find -lfl
```

解决方法：

- 在 `expr.l` 中加入 `%option noyywrap`；
- 删除手写 `yywrap` 函数；
- 删除 `Makefile` 中的 `-lfl` 链接选项。

修复后，Flex/Bison 选做部分成功运行。

### 3. Lab4 网页实现

实验四完成后，我把实验 1-4 整合到一个完整 HTML 页面中。

新增的 SLR(1) 页面包括：

- 文法输入；
- FIRST 集展示；
- FOLLOW 集展示；
- LR(0) 项目集族展示；
- SLR(1) ACTION 表展示；
- SLR(1) GOTO 表展示；
- 冲突检测结果；
- 输入串分析过程；
- JSON 输出；
- Flex/Bison 选做说明。

其中，`slr1_result.json`、`action.csv`、`goto.csv` 也可以作为后续 UI 接入或数据展示的基础。

---

# 综合 UI 页面说明

## 页面目标

综合 UI 页面用于将 Lab1-Lab4 的核心过程集中展示，体现编译器前端从字符识别到语法分析表生成的完整流程：

```text
DFA 字符串识别
        ↓
类 C 语言词法分析
        ↓
LR(0) 项目集规范族
        ↓
SLR(1) ACTION/GOTO 分析表
```

## 页面技术

页面采用纯前端实现：

```text
HTML + CSS + JavaScript
```

不依赖后端服务，直接用浏览器打开即可使用。

如果放在服务器上，也可以通过 Python 临时 HTTP 服务访问：

```bash
cd lab4/ui
python3 -m http.server 8080
```

浏览器访问：

```text
http://服务器IP:8080/compiler_labs_1_2_3_4_integrated.html
```

如果服务器端口不能直接访问，可以使用 SSH 端口转发：

```bash
ssh -L 8080:127.0.0.1:8080 root@服务器IP
```

然后访问：

```text
http://127.0.0.1:8080/compiler_labs_1_2_3_4_integrated.html
```

## 页面模块

### 1. DFA 可视化实验

实现功能：

- 内置“以 ab 结尾”案例；
- 内置“包含 to”案例；
- 支持自定义 DFA；
- 展示 DFA 五元组；
- 判定输入字符串；
- 枚举合法字符串；
- 检查 DFA 合法性；
- 显示状态转移表。

### 2. 类 C 词法分析器

实现功能：

- 输入类 C 源代码；
- 识别关键字、标识符、整数、实数、运算符、界符；
- 输出 token 表格；
- 显示 token 类型、属性值、行号、列号；
- 显示词法错误；
- 统计 token 数量。

### 3. LR(0) 项目集规范族

实现功能：

- 输入表达式文法；
- 自动增广文法；
- 计算 Closure；
- 计算 Goto；
- 生成项目集规范族；
- 展示 Kernel Items 和 Closure Items；
- 输出 Goto 表；
- 绘制状态图；
- 检测 LR(0) 冲突；
- 输出 JSON 数据。

### 4. SLR(1) 分析表

实现功能：

- 输入表达式文法；
- 计算 FIRST 集和 FOLLOW 集；
- 复用 LR(0) 项目集规范族；
- 根据移进项目生成 ACTION 中的 `s` 动作；
- 根据完整项目和 FOLLOW 集生成 ACTION 中的 `r` 动作；
- 根据增广产生式生成 `acc`；
- 生成 GOTO 表；
- 检测冲突；
- 展示输入串分析过程。

---

# 编译和运行方式

## Lab1

```bash
cd lab1
g++ -std=c++17 -O2 -Wall -Wextra -o dfa main.cpp
./dfa
```

## Lab2

```bash
cd lab2
g++ -std=c++17 -O2 -Wall -Wextra -o scanner scanner.cpp
./scanner
```

Flex 选做：

```bash
cd lab2
flex flex_scanner.l
gcc lex.yy.c -o flex_scanner
./flex_scanner
```

## Lab3

```bash
cd lab3
make clean
make run
```

如果需要生成状态图：

```bash
dot -Tpng out/lr0_graph.dot -o out/lr0_graph.png
```

## Lab4

```bash
cd lab4
make clean
make
make run
make parse
```

也可以直接运行：

```bash
./slr1 --grammar examples/grammar_expr.txt --out-dir out --parse "id + id * id"
```

Flex/Bison 选做：

```bash
cd lab4/optional/flex_bison
make clean
make
echo "id + id * id" | ./expr_parser
```

如果服务器缺少 `libfl`，使用 `%option noyywrap` 并删除 `-lfl` 后重新编译。

## UI 页面

本地直接打开：

```text
lab4/ui/compiler_labs_1_2_3_4_integrated.html
```

服务器访问：

```bash
cd lab4/ui
python3 -m http.server 8080
```

---

# 输出文件说明

## Lab2

```text
token_output.txt
```

保存类 C 源程序扫描后的 token 序列，可作为后续语法分析输入。

## Lab3

```text
out/lr0_collection.json
out/lr0_graph.dot
out/lr0_graph.png
out/token.txt
```

保存 LR(0) 项目集、状态图和 token 结果。

## Lab4

```text
out/report.txt
out/action.csv
out/goto.csv
out/slr1_result.json
```

其中：

- `report.txt`：完整文本报告输出；
- `action.csv`：SLR(1) ACTION 表；
- `goto.csv`：SLR(1) GOTO 表；
- `slr1_result.json`：供 UI 页面读取或展示的结构化结果。

---

# GitHub 上传建议

建议上传内容：

```text
lab1/
lab2/
lab3/
lab4/
README.md
```

建议不要上传中间编译产物，例如：

```text
*.o
*.exe
lex.yy.c
expr.tab.c
expr.tab.h
expr_parser
```

可以使用 `.gitignore`：

```gitignore
*.o
*.out
*.exe
*.log
.DS_Store
.vscode/
__pycache__/
node_modules/

lab4/optional/flex_bison/expr.tab.c
lab4/optional/flex_bison/expr.tab.h
lab4/optional/flex_bison/lex.yy.c
lab4/optional/flex_bison/expr_parser
```

上传命令示例：

```bash
git init
git add .
git commit -m "upload compiler labs 1-4"
git branch -M main
git remote add origin https://github.com/你的用户名/compiler-labs.git
git push -u origin main
```

---

# 总结

本项目完成了编译原理前端阶段的连续实验：

1. 实验一通过 DFA 完成字符串识别；
2. 实验二在 DFA 思想基础上完成类 C 语言 token 扫描；
3. 实验三使用词法分析结果作为语法分析输入，构造 LR(0) 项目集规范族；
4. 实验四在 LR(0) 规范族和 FOLLOW 集基础上生成 SLR(1) 分析表，并完成输入串分析。

综合 UI 页面把四次实验的关键结果统一展示出来，使每个实验既可以独立运行，也可以串联成一个完整的编译器前端流程。
