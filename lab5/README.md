# 实验五：SLR 引导的语义分析框架完整代码包

本工程根据实验四材料继续扩展，面向实验五完成：

- 词法分析
- 语法分析入口
- 语义动作框架
- 属性栈接口
- AST 构建
- 符号表维护
- 作用域管理
- 类型检查
- 语义错误报告
- AST 文本树与 Graphviz DOT 输出
- 对 1.src 到 12.src 示例程序的批量测试


---

## 目录结构

```txt
lab5_slr_semantic_complete/
├── Makefile                         # 实验五主程序编译入口
├── CMakeLists.txt                    # CMake 编译入口
├── include/
│   ├── ast.h                         # AST 节点与输出接口
│   ├── lexer.h                       # 词法分析器
│   ├── parser.h                      # 示例源程序语法分析器
│   ├── semantic.h                    # 语义上下文、类型检查接口
│   ├── symbols.h                     # 符号表与作用域
│   ├── token.h                       # Token 定义
│   └── slr_semantic_adapter.h        # 实验四 SLR reduce 接实验五语义动作的适配层
├── src/
│   ├── ast.cpp
│   ├── lexer.cpp
│   ├── parser.cpp
│   ├── semantic.cpp
│   ├── symbols.cpp
│   ├── token.cpp
│   ├── slr_semantic_adapter.cpp
│   └── main.cpp
├── examples/
│   ├── 1.src ... 12.src              # 示例程序
├── scripts/
│   └── run_all.sh                    # 批量运行示例
├── docs/
│   ├── slr_integration.md            # 如何把实验五接到实验四 SLR 主循环
│   ├── experiment4_uploaded_summary.md
│   └── report_template.md            # 实验报告写法模板
├── lab4_slr1_reconstructed/          # 重新实现的实验四 SLR(1) 表生成器
│   ├── Makefile
│   ├── src/main.cpp
│   └── examples/
└── lab4_reference/                   # 实验四原始文件备份
```

---

## 一、实验五主程序运行

编译：

```bash
make
```

运行单个示例：

```bash
./lab5_semantic examples/1.src -o output
```

批量运行示例：

```bash
./scripts/run_all.sh
```

或：

```bash
make run
```

输出文件：

```txt
output/1.ast.txt       # AST 文本树
output/1.ast.dot       # Graphviz DOT，可转图片
output/1.symbols.txt   # 符号表
output/1.errors.txt    # 语义错误和 warning
```

Graphviz 转图：

```bash
dot -Tpng output/1.ast.dot -o output/1.ast.png
```

---

## 二、实验四 SLR(1) 表生成器运行

```bash
cd lab4_slr1_reconstructed
make
make parse
```

输出：

```txt
lab4_slr1_reconstructed/out/report.txt
lab4_slr1_reconstructed/out/action.csv
lab4_slr1_reconstructed/out/goto.csv
lab4_slr1_reconstructed/out/parse_steps.txt
```

这部分用于证明你实验四的 SLR(1) 分析表生成逻辑，也可以作为实验五“SLR 规约时执行语义动作”的基础。

---

## 三、已经实现的实验五必做内容

1. 文法相关程序结构和语义动作框架
2. 属性对象 `Attribute`
3. AST 节点 `ASTNode`
4. 符号表 `SymbolTable`
5. 作用域栈
6. 变量声明检查
7. 数组声明和数组访问检查
8. 函数声明和函数调用检查
9. 参数个数检查
10. 参数类型检查
11. 赋值类型检查
12. return 类型检查
13. 未声明变量/函数检测
14. 重复声明检测
15. AST 文本树输出
16. AST DOT 可视化输出
17. 语义错误报告

---

## 四、有意义的选做拓展

1. AST 节点保留 `place` 字段，为实验六生成四元式或三地址码做准备。
2. 表达式生成 `t1`、`t2` 等临时变量名。
3. 输出 Graphviz DOT 格式 AST。
4. 对 `&&`、`||`、`!`、一元负号提供拓展支持，并在错误报告中以 warning 标出。
5. 单独保留实验四 SLR(1) 表生成器，便于和实验五语义动作对接。

---

## 五、严格接入你实验四 SLR 主循环的方法

实验四主循环中：

```cpp
if (ACTION[state][lookahead] == shift) {
    adapter.onShift(token);
}

if (ACTION[state][lookahead] == reduce) {
    adapter.onReduce(production);
}
```

本包已经提供 `SLRSemanticAdapter`，你只需要把实验四的产生式编号、左部、右部传进去即可。详细说明看：

```txt
docs/slr_integration.md
include/slr_semantic_adapter.h
src/slr_semantic_adapter.cpp
```
