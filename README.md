# 编译原理实验合集Lab1-Lab5

本仓库整理了编译原理课程前五次实验的代码、测试文件、运行结果与综合可视化网页。五次实验从DFA字符串识别开始，逐步扩展到类C语言词法分析、LR(0)项目集规范族构造、SLR(1)分析表生成，最后完成SLR引导的语义分析框架，实现AST构建、符号表维护、语义错误检测、AST可视化、批量测试汇总和初步四元式IR生成。通过HTML页面可以统一展示Lab1-Lab5的核心过程和运行结果。

## 项目概述

| 实验 | 主题 | 核心目标 | 完成情况 |
|---|---|---|---|
| Lab1 | DFA模拟程序设计 | 读取DFA五元组，检查DFA合法性，枚举合法字符串，模拟字符串识别过程 | 已完成必做、选做和DFA可视化页面 |
| Lab2 | 类C语言词法分析器Scanner | 使用DFA/手写扫描器识别类C语言token，输出token序列 | 已完成必做、Flex选做和Scanner可视化页面 |
| Lab3 | LR(0)项目集规范族构造 | 输入文法，增广文法，计算Closure/Goto，生成LR(0)项目集规范族 | 已完成必做、冲突检测、状态图和Web UI |
| Lab4 | SLR(1)分析表生成 | 基于LR(0)规范族和FOLLOW集生成ACTION/GOTO表，并完成输入串分析 | 已完成必做、Flex/Bison选做和Lab1-Lab4综合UI页面 |
| Lab5 | SLR引导的语义分析框架 | 在语法分析基础上嵌入语义动作，构建AST，维护符号表，检测语义错误，并输出初步四元式IR | 已完成必做、AST可视化、批量测试汇总、IR生成和Lab5 UI页面 |

## 实验环境

主要开发和运行环境如下：

```text
操作系统：openEuler/Linux云端环境
运行平台：鲲鹏服务器/ARM64
开发方式：VS Code+SSH远程连接/服务器终端
主要语言：C/C++/HTML/CSS/JavaScript
编译工具：gcc/g++/make
选做工具：flex/bison/Graphviz
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
├── lab5/
│   ├── Makefile
│   ├── include/
│   │   ├── ast.h
│   │   ├── lexer.h
│   │   ├── parser.h
│   │   ├── semantic.h
│   │   ├── symbols.h
│   │   ├── token.h
│   │   └── ir.h
│   ├── src/
│   │   ├── ast.cpp
│   │   ├── lexer.cpp
│   │   ├── parser.cpp
│   │   ├── semantic.cpp
│   │   ├── symbols.cpp
│   │   ├── token.cpp
│   │   ├── ir.cpp
│   │   └── main.cpp
│   ├── examples/
│   │   ├── 1.src
│   │   ├── 2.src
│   │   └── ...
│   ├── scripts/
│   │   ├── run_all.sh
│   │   └── summary.sh
│   ├── output/
│   │   ├── summary.csv
│   │   ├── 1.ast.txt
│   │   ├── 1.ast.dot
│   │   ├── 1.ast.png
│   │   ├── 1.symbols.txt
│   │   ├── 1.errors.txt
│   │   └── 1.ir.txt
│   └── ui/
│       └── lab5.html
└── docs/
```

实际目录可以根据自己的服务器目录调整，只要保证每个实验的源码、输入文件、输出文件和运行说明完整即可。

---

# Lab1：DFA模拟程序设计

## 实验要求

实验一要求编写DFA模拟程序，能够从文本文件读取DFA五元组，包括：

- 字符集；
- 状态集；
- 开始状态；
- 接受状态集；
- 状态转移函数。

程序还需要完成DFA合法性检查，并实现两个核心功能：

1. 输入最大长度`N`，输出该DFA语言中长度不超过`N`的所有合法字符串；
2. 输入一个字符串，模拟DFA的识别过程，输出完整状态转移路径，并判断该字符串是否属于该语言。

## 我完成的内容

### 1. 必做部分

我实现了一个C++版本的DFA模拟程序，使用文本文件保存DFA五元组。

必做案例为：识别所有以`ab`结尾的字符串。

对应语言为：

```text
L={w|w以ab结尾}
```

DFA状态设计如下：

```text
q0：初始状态，尚未形成后缀ab
q1：已经读到字符a
q2：已经形成后缀ab，接受状态
```

输入文件为`dfa_basic.txt`，主要内容包括：

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

- 读取DFA文件；
- 输出DFA基本信息；
- 检查开始状态是否合法；
- 检查接受状态是否为空、是否属于状态集；
- 检查转移函数是否完整；
- 枚举长度不超过`N`的合法字符串；
- 对输入串进行逐步识别；
- 对非法字符串、非法字符、错误DFA文件进行测试。

### 2. 选做部分

选做部分我结合教材示例，设计了一个识别包含子串`to`的DFA。

对应语言为：

```text
L={w|w中含有子串to}
```

状态设计如下：

```text
q0：初始状态，尚未读到t
q1：已经读到t
q2：已经匹配到子串to，接受状态
```

输入文件为`dfa_to.txt`，程序可以读取该文件，并判断输入字符串是否包含子串`to`。

### 3. Lab1网页实现

我为实验一实现了DFA可视化页面，主要功能包括：

- 必做案例和选做案例切换；
- DFA五元组展示；
- 状态转移表展示；
- 输入字符串判定；
- 长度不超过`N`的合法字符串枚举；
- 自定义DFA输入；
- 模拟终端输出运行过程。

---

# Lab2：类C语言词法分析器Scanner

## 实验要求

实验二要求实现一个类C语言词法分析器scanner。词法分析器需要把源程序中的字符序列转换为token序列，为后续语法分析提供输入。

实验要求包括：

1. 对若干个独立符号串进行分类；
2. 对一整行类C代码进行扫描；
3. 能够识别主要词法单位，例如标识符、整数、实数、关键字、算术运算符、关系运算符、赋值符号、括号和分号等；
4. 能够将token序列输出到文件，作为后续语法分析输入。

## 我完成的内容

### 1. 必做部分

我实现了一个手写C++ scanner，支持三种运行模式：

```text
模式1：对多个独立字符串进行分类
模式2：对一行类C代码进行扫描
模式3：读取test.c文件并输出token_output.txt
```

识别的token类型包括：

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

- 支持`//`单行注释跳过；
- 支持词法错误定位；
- 遇到非法字符后不直接退出，而是记录错误并继续分析；
- 将`test.c`扫描结果写入`token_output.txt`；
- token输出可以作为后续语法分析实验的输入。

### 2. 选做部分

选做部分我使用`flex`实现了基于正则式的词法分析器。

主要正则式包括：

```text
DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INTNUM      [+-]?{DIGIT}+
FLOATNUM    [+-]?{DIGIT}*"."{DIGIT}+
```

完成内容：

- 编写`flex_scanner.l`；
- 使用`flex`生成`lex.yy.c`；
- 使用`gcc`编译生成可执行程序；
- 使用单行代码测试；
- 使用`test.c`测试；
- 将输出保存到`flex_output.txt`；
- 对比flex版本和手写scanner版本的输出结果。

### 3. Lab2网页实现

在网页中，我将实验一DFA模块和实验二scanner模块整合到一个页面中。

Scanner页面功能包括：

- 输入类C源代码；
- 点击按钮执行词法分析；
- 表格输出token类型、属性值、行号和列号；
- 显示词法错误；
- 统计各类token数量；
- 加载示例代码；
- 清空分析结果。

该页面体现了DFA与scanner的关系：正则式描述词法规则，DFA识别字符串，scanner则把多个token的识别过程组合起来处理源程序。

---

# Lab3：LR(0)项目集规范族构造

## 实验要求

实验三要求构造给定文法的LR(0)项目集规范族，为后续生成SLR(1)分析表提供基础。

主要要求包括：

1. 输入文法规则；
2. 支持扩展巴科斯范式；
3. 增广文法；
4. 从初始项目开始构造I0；
5. 计算Closure闭包；
6. 计算Goto状态转移；
7. 重复构造新的项目集，直到不再产生新状态；
8. 输出所有LR(0)项目集；
9. 输出状态转移关系；
10. 检测移进-归约冲突或归约-归约冲突。

## 我完成的内容

### 1. 必做部分

我实现了LR(0)项目集规范族生成程序。

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
- Closure闭包计算；
- Goto转移计算；
- LR(0)项目集规范族生成；
- 内核项展示；
- 状态转移关系输出；
- LR(0)冲突检测；
- 输出JSON文件；
- 输出Graphviz`.dot`文件；
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

原因是这些状态中同时存在归约项目和移进项目，说明该表达式文法不是严格LR(0)文法，但可以继续作为SLR(1)分析表构造的基础。

### 2. 选做部分

选做部分我完成了实验1、实验2、实验3的UI串联。

主要包括：

- DFA可视化实验；
- 类C词法分析器；
- LR(0)项目集规范族构造；
- 实验二token输出和实验三语法分析输入的联系展示；
- 项目集卡片展示；
- Goto表展示；
- 状态图展示；
- JSON输出展示。

### 3. Lab3网页实现

LR(0)页面可以直接输入文法，点击按钮后自动展示：

- 增广文法；
- 产生式数量；
- 状态数量；
- 转移数量；
- 冲突数量；
- 所有项目集；
- Kernel Items；
- Closure Items；
- Goto转移；
- Goto表；
- 状态图；
- JSON数据。

该页面让命令行中的Closure、Goto和项目集构造过程变得更加直观。

---

# Lab4：SLR(1)分析表生成

## 实验要求

实验四要求在实验三LR(0)项目集规范族的基础上生成SLR(1)分析表。

输入包括：

```text
实验三生成的LR(0)规范族
文法符号的FOLLOW集
```

输出为：

```text
SLR(1)分析表，即ACTION/GOTO二维表
```

SLR(1)与LR(0)的主要区别是：SLR(1)在归约时引入FOLLOW集，只在当前输入符号属于产生式左部非终结符的FOLLOW集时才进行归约，从而减少LR(0)中的部分冲突。

## 我完成的内容

### 1. 必做部分

我实现了C++17版本的SLR(1)分析表生成程序。

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
- 计算FIRST集；
- 计算FOLLOW集；
- 构造LR(0)项目集规范族；
- 输出GOTO转换关系；
- 生成SLR(1)ACTION表；
- 生成SLR(1)GOTO表；
- 检测移进-归约冲突和归约-归约冲突；
- 对输入串进行SLR(1)分析；
- 输出分析过程；
- 导出`report.txt`、`action.csv`、`goto.csv`、`slr1_result.json`。

必做程序运行后，表达式文法无冲突：

```text
无冲突：该文法在当前构造下是SLR(1)文法。
```

输入串测试：

```text
id + id * id
```

最终分析到：

```text
acc，分析成功
```

说明SLR(1)分析表生成和输入串分析过程都正确。

### 2. 选做部分

选做部分我完成了Flex/Bison表达式语法分析器。

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

- 使用`flex`编写词法规则；
- 使用`bison`编写表达式语法规则；
- 生成`expr.tab.c`、`expr.tab.h`、`lex.yy.c`；
- 编译生成`expr_parser`；
- 测试表达式`id + id * id`；
- 输出`语法分析成功`。

运行过程中，鲲鹏服务器缺少`libfl`，编译时报错：

```text
/usr/bin/ld: cannot find -lfl
```

解决方法：

- 在`expr.l`中加入`%option noyywrap`；
- 删除手写`yywrap`函数；
- 删除`Makefile`中的`-lfl`链接选项。

修复后，Flex/Bison选做部分成功运行。

### 3. Lab4网页实现

实验四完成后，我把实验1-4整合到一个完整HTML页面中。

新增的SLR(1)页面包括：

- 文法输入；
- FIRST集展示；
- FOLLOW集展示；
- LR(0)项目集族展示；
- SLR(1)ACTION表展示；
- SLR(1)GOTO表展示；
- 冲突检测结果；
- 输入串分析过程；
- JSON输出；
- Flex/Bison选做说明。

其中，`slr1_result.json`、`action.csv`、`goto.csv`也可以作为后续UI接入或数据展示的基础。

---

# Lab5：SLR引导的语义分析框架实现

## 实验要求

实验五要求在SLR(1)语法分析基础上集成语义动作，构建抽象语法树AST并维护符号表，完成类型检查和初步语义验证。

实验五的主要输入包括：

```text
SLR(1)分析表
嵌入语义动作的文法规则
实验二输出的token流
```

实验五的主要输出包括：

```text
AST抽象语法树
符号表
语义错误报告
```

在完成必做要求的基础上，我还扩展了AST可视化、批量测试汇总、初步四元式IR生成和Lab5专用UI页面，用于展示语义分析结果。

## 我完成的内容

### 1. 必做部分

我实现了一个C++17版本的语义分析框架。程序可以读取`.src`源程序，完成词法分析、语法分析、语义分析，并输出AST、符号表和语义错误报告。

核心数据结构包括：

```text
ASTNode：表示抽象语法树节点
Symbol：表示符号表项
SymbolTable：维护变量、数组、函数及作用域信息
SemanticContext：记录符号表、语义错误和警告
IRGenerator：在AST基础上生成四元式IR
```

必做功能包括：

- 识别变量声明；
- 识别函数声明；
- 识别数组声明；
- 识别赋值语句；
- 识别return语句；
- 识别print语句；
- 识别函数调用；
- 构建AST文本树；
- 维护符号表；
- 支持作用域记录；
- 检测重复声明；
- 检测未声明变量；
- 检测未声明函数；
- 检测函数参数数量不匹配；
- 检测部分类型不匹配；
- 输出语义错误报告。

单个样例运行方式：

```bash
cd lab5
make clean
make
./lab5_semantic examples/1.src -o output
```

生成文件示例：

```text
output/1.ast.txt
output/1.ast.dot
output/1.symbols.txt
output/1.errors.txt
output/1.ir.txt
```

### 2. 选做部分一：AST可视化

在AST文本树的基础上，程序进一步输出Graphviz DOT文件，例如：

```text
output/1.ast.dot
output/12.ast.dot
```

使用Graphviz可以生成PNG图片：

```bash
dot -Tpng output/1.ast.dot -o output/1.ast.png
dot -Tpng output/12.ast.dot -o output/12.ast.png
```

该功能可以直观展示程序结构，例如函数声明、变量声明、赋值语句、return语句、表达式和函数调用之间的父子关系。

### 3. 选做部分二：批量测试和汇总

为了验证语义分析框架的稳定性，我编写了批量测试脚本，对`examples/`目录下的多个`.src`示例程序逐一运行：

```bash
make run
```

同时编写了`summary.sh`，自动统计每个样例的输出情况：

```bash
./scripts/summary.sh
```

汇总结果保存为：

```text
output/summary.csv
```

汇总表字段包括：

```text
case,ast,dot,png,ir,symbols,semantic_errors,warnings,status
```

其中：

- `ast`表示是否生成AST文本；
- `dot`表示是否生成AST DOT文件；
- `png`表示是否生成AST图片；
- `ir`表示是否生成四元式IR；
- `symbols`表示是否生成符号表；
- `semantic_errors`表示语义错误数量；
- `warnings`表示警告数量；
- `status`表示样例整体状态。

### 4. 选做部分三：初步四元式IR生成

为衔接实验六中间代码生成，我在Lab5中加入了初步四元式IR生成模块。该模块在AST基础上遍历语句和表达式，输出类似三地址码的四元式。

四元式格式为：

```text
序号: (op,arg1,arg2,result)
```

示例输出：

```text
0: (func,main,int,_)
1: (decl,int,_,x)
2: (=,5,_,x)
3: (return,0,_,_)
4: (endfunc,main,_,_)
5: (call,main,0,t1)
```

当前IR生成支持：

- 函数开始和结束；
- 参数声明；
- 变量声明；
- 数组声明；
- 赋值语句；
- 复合赋值；
- 自增语句；
- return语句；
- print语句；
- 函数调用；
- 算术表达式；
- 一元运算；
- if语句；
- while语句。

该部分虽然仍属于初步中间代码生成，但已经可以作为Lab6的基础。

### 5. Lab5网页实现

我为实验五单独实现了一个HTML可视化页面`lab5.html`，用于展示语义分析结果。

页面功能包括：

- 自动读取`output/summary.csv`；
- 显示样例总数；
- 显示IR文件数量；
- 显示AST PNG数量；
- 显示错误样例数量；
- 展示批量测试汇总表；
- 按样例查看AST文本；
- 按样例查看符号表；
- 按样例查看语义错误报告；
- 按样例查看四元式IR；
- 按样例查看AST图片；
- 支持自动读取文件；
- 支持手动上传文件展示；
- 提供报告亮点说明页面。

运行方式：

```bash
cd lab5
python3 -m http.server 8000
```

浏览器访问：

```text
http://服务器IP:8000/lab5.html
```

如果服务器端口不能直接访问，可以使用SSH端口转发：

```bash
ssh -L 8000:127.0.0.1:8000 root@服务器IP
```

然后在本地浏览器访问：

```text
http://127.0.0.1:8000/lab5.html
```

Lab5 UI页面将AST、符号表、错误报告、IR和AST图片集中展示，使语义分析阶段的结果更加直观，也便于验收展示。

---

# 综合UI页面说明

## 页面目标

综合UI页面用于将Lab1-Lab5的核心过程集中展示，体现编译器前端从字符识别到语义分析的完整流程：

```text
DFA字符串识别
        ↓
类C语言词法分析
        ↓
LR(0)项目集规范族
        ↓
SLR(1)ACTION/GOTO分析表
        ↓
SLR引导的语义分析
```

## 页面技术

页面采用纯前端实现：

```text
HTML+CSS+JavaScript
```

Lab1-Lab4综合页面主要用于动态构造和展示DFA、Scanner、LR(0)项目集和SLR(1)分析表。Lab5页面主要读取服务器端已经生成的输出文件，并展示AST、符号表、语义错误报告、四元式IR和AST图片。

如果放在服务器上，也可以通过Python临时HTTP服务访问：

```bash
cd lab5
python3 -m http.server 8000
```

浏览器访问：

```text
http://服务器IP:8000/lab5.html
```

如果服务器端口不能直接访问，可以使用SSH端口转发：

```bash
ssh -L 8000:127.0.0.1:8000 root@服务器IP
```

然后访问：

```text
http://127.0.0.1:8000/lab5.html
```

## 页面模块

### 1. DFA可视化实验

实现功能：

- 内置“以ab结尾”案例；
- 内置“包含to”案例；
- 支持自定义DFA；
- 展示DFA五元组；
- 判定输入字符串；
- 枚举合法字符串；
- 检查DFA合法性；
- 显示状态转移表。

### 2. 类C词法分析器

实现功能：

- 输入类C源代码；
- 识别关键字、标识符、整数、实数、运算符、界符；
- 输出token表格；
- 显示token类型、属性值、行号、列号；
- 显示词法错误；
- 统计token数量。

### 3. LR(0)项目集规范族

实现功能：

- 输入表达式文法；
- 自动增广文法；
- 计算Closure；
- 计算Goto；
- 生成项目集规范族；
- 展示Kernel Items和Closure Items；
- 输出Goto表；
- 绘制状态图；
- 检测LR(0)冲突；
- 输出JSON数据。

### 4. SLR(1)分析表

实现功能：

- 输入表达式文法；
- 计算FIRST集和FOLLOW集；
- 复用LR(0)项目集规范族；
- 根据移进项目生成ACTION中的`s`动作；
- 根据完整项目和FOLLOW集生成ACTION中的`r`动作；
- 根据增广产生式生成`acc`；
- 生成GOTO表；
- 检测冲突；
- 展示输入串分析过程。

### 5. Lab5语义分析页面

实现功能：

- 自动读取`output/summary.csv`；
- 展示批量测试汇总表；
- 展示样例AST文本；
- 展示样例符号表；
- 展示样例错误报告；
- 展示样例四元式IR；
- 展示样例AST PNG图片；
- 支持手动上传结果文件；
- 展示必做和选做亮点。

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

Flex选做：

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

Flex/Bison选做：

```bash
cd lab4/optional/flex_bison
make clean
make
echo "id + id * id" | ./expr_parser
```

如果服务器缺少`libfl`，使用`%option noyywrap`并删除`-lfl`后重新编译。

## Lab5

```bash
cd lab5
make clean
make
./lab5_semantic examples/1.src -o output
```

批量运行：

```bash
make run
```

生成汇总表：

```bash
./scripts/summary.sh
```

生成AST图片：

```bash
for f in output/*.ast.dot; do
    dot -Tpng "$f" -o "${f%.dot}.png"
done
```

重新生成汇总表，让`png`列显示`yes`：

```bash
./scripts/summary.sh
```

启动Lab5 UI页面：

```bash
python3 -m http.server 8000
```

浏览器访问：

```text
http://服务器IP:8000/lab5.html
```

## UI页面

Lab1-Lab4综合页面：

```text
lab4/ui/compiler_labs_1_2_3_4_integrated.html
```

Lab5语义分析页面：

```text
lab5/ui/lab5.html
```

服务器访问Lab5页面：

```bash
cd lab5
python3 -m http.server 8000
```

---

# 输出文件说明

## Lab2

```text
token_output.txt
```

保存类C源程序扫描后的token序列，可作为后续语法分析输入。

## Lab3

```text
out/lr0_collection.json
out/lr0_graph.dot
out/lr0_graph.png
out/token.txt
```

保存LR(0)项目集、状态图和token结果。

## Lab4

```text
out/report.txt
out/action.csv
out/goto.csv
out/slr1_result.json
```

其中：

- `report.txt`：完整文本报告输出；
- `action.csv`：SLR(1)ACTION表；
- `goto.csv`：SLR(1)GOTO表；
- `slr1_result.json`：供UI页面读取或展示的结构化结果。

## Lab5

```text
output/summary.csv
output/*.ast.txt
output/*.ast.dot
output/*.ast.png
output/*.symbols.txt
output/*.errors.txt
output/*.ir.txt
```

其中：

- `summary.csv`：批量测试汇总结果；
- `*.ast.txt`：AST文本树；
- `*.ast.dot`：Graphviz格式AST描述；
- `*.ast.png`：AST图片；
- `*.symbols.txt`：符号表；
- `*.errors.txt`：语义错误和警告报告；
- `*.ir.txt`：四元式IR输出。

---

# 总结

本项目完成了编译原理前端阶段的连续实验：

1. 实验一通过DFA完成字符串识别；
2. 实验二在DFA思想基础上完成类C语言token扫描；
3. 实验三使用词法分析结果作为语法分析输入，构造LR(0)项目集规范族；
4. 实验四在LR(0)规范族和FOLLOW集基础上生成SLR(1)分析表，并完成输入串分析；
5. 实验五在SLR(1)语法分析基础上加入语义动作，完成AST构建、符号表维护、语义错误检测、AST可视化、批量测试汇总和初步四元式IR生成。

综合UI页面把五次实验的关键结果统一展示出来，使每个实验既可以独立运行，也可以串联成一个完整的编译器前端流程：

```text
字符识别
  → 词法分析
  → 语法分析项目集构造
  → SLR(1)分析表生成
  → 语义分析
  → 初步中间代码IR生成
```
