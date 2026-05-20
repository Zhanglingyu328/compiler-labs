# 实验四上传材料说明

你本次上传的实验四材料包括：

1. `dfa(1).html`：综合可视化页面，包含 DFA、词法分析、LR(0) 项目集、SLR(1) 分析表模块。
2. `README(3).md`：实验四项目说明，描述了 SLR(1) 表生成器的输入、输出和运行方式。
3. `Makefile(1)`：实验四命令行项目 Makefile。
4. `slr1`：已编译的可执行文件。

其中 `slr1` 是 ELF 64-bit aarch64 Linux 可执行程序，不是 C++ 源码，所以本包在 `lab4_slr1_reconstructed/` 中重新写了一份可编译、可运行的 C++17 版 SLR(1) 表生成器。

实验五主程序没有直接依赖这个二进制，而是把实验四需要的 SLR shift/reduce 对接方式抽象成了：

```cpp
SLRSemanticAdapter::onShift(token)
SLRSemanticAdapter::onReduce(production)
```

这样你既可以直接跑实验五示例，也可以把语义动作接回你自己的实验四 SLR 主循环。
