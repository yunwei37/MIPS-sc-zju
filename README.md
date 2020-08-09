# MIPS-sc
MIPS-sc 为 MIPS simulator&compiler 的简称，是一个基于Qt实现的带图形界面的MIPS汇编指令的编辑器、汇编器、反汇编器、模拟器。是为浙江大学《计算机组成课程》编写的的课程项目之一。

## 预览
- 模拟器界面:

左侧为32位内存内容、可以以ascii码方式或反汇编指令方式查看；

右侧为寄存器、syscall输入输出窗口，可通过按钮查看相应寄存器与内存的值、修改PC值、对应内存单元值；

![image](https://github.com/yunwei37/MIPS-sc/raw/master/document/sim-eg.png)

- 文件菜单：

新建、打开、保存代码文件；

支持 .asm 汇编文件汇编为 .bin 文件并导出；

支持  .bin 文件反汇编为 .asm 文件，支持导入加载 .bin 文件并执行：

![image](https://github.com/yunwei37/MIPS-sc/raw/master/document/memu-file.png)

- 编辑器/编译界面

上方为代码编辑器，下方为编译输出结果，对相应错误有错误详细信息以及行号提示

![image](https://github.com/yunwei37/MIPS-sc/raw/master/document/com-eg.png)

## 特性
- 支持语法高亮；
- 支持部分伪指令；
- 支持 .asm 汇编文件汇编为 .bin 文件；
- 支持  .bin 文件反汇编为 .asm 文件，支持加载 .bin 文件并执行：
- 支持模拟运行机器码，支持模拟终端输入输出
- 支持简单的调试功能：单步运行、连续运行、设置断点、查看寄存器与内存的值、修改PC值、对应内存单元值

## 指令集
参考《ZPC之MIPS指令集2019》

- R指令：
    add slt sltu and or xor nor sllv srlv srav mul mfhi mflo
    mtlo subu
- I指令：
    slti sltiu addi addiu andi ori xori sub sw sh lw lh lhu bne beq bgez bgtz blez bltz lb lhu lui
- J指令：
    j jal 
- syscall功能

## 模拟
MIPS-sim 具有简单的模拟与调试功能。可以通过 syscall 指令向终端输出信息，或从终端读入用户输入信息。在编辑器输入代码后可以使用 ”simulate“ 按键进行编译和将机器码加载到内存，可以通过step按键单步执行内存中的指令、或设置断点进行连续执行，代码将会执行至断点处停止。

## 项目
- project：Qt项目源代码文件
- test：测试用例
- document：文档

运行时需要将code.txt代码配置文档放在程序运行目录

## 源代码
由三个部分组成，其中模拟器类和汇编、反汇编c语言库可复用

1. C语言实现的将MIPS汇编指令转换为机器码，或进行反汇编
    - 公共头文件compiler.h: 

	    compile.c 实现了将输入的源代码文件转换为二进制数组；

        decompiler.c 将一条机器码反汇编为MIPS汇编指令

        singleCompiler.c 将一条MIPS汇编指令编译为机器码（可单独调用）

2. c++实现的模拟器类：

	- simulator.h

        simulator.cpp

3.  Qt窗口类：

    - mainwindow.h

        mainwindow.cpp

    - highlighter.h

        highlighter.cpp 代码高亮类
        
    - main.h （作为程序入口文件）

