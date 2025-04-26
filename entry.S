#include "asm.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"


# xv6 内核在此文件开始执行，文件与内核 C 代码链接，可以引用内核符号
# 引导块(boot_first_cpu.S 和 bootmain.c)跳转到 entry 标签处


# Multiboot header  引导 Multiboot 加载器的数据
.p2align 2                          # 强制 4-byte 对齐
.text                               # 声明代码段
.globl multiboot_header             # 声明全局变量

# .long —— 4-byte
multiboot_header:
    #define magic 0x1badb002
    #define flags 0
    .long magic                     # magic should be the same as magic defined      
    .long flags               
        # 引导加载器 do: sum = magic + flags + checksum == 0 -> 头部信息未被破坏  
    .long (-magic-flags)            # checksum

# _start 指定 ELF 入口
# entry 虚拟地址通过 V2P_WO 宏转为物理地址并赋给 _start
.globl _start
_start = V2P_WO(entry)

.globl entry
entry:
# 开启 4MB 页面大小扩展
        # %CR4 包含与处理器功能和操作模式相关的标志位
    movl    %cr4, %eax              # (%cr4) -> (%eax)
        # CR4_PSE(页面大小扩展标志位)置 1，开启 4MB 页面大小扩展
    orl     $(CR4_PSE), %eax        # (%eax) = ($CR0_PE) | (%eax)
    movl    %eax, %cr4              # (%eax) -> (%cr4) 

# 设置页目录
        # 将 entrypgdir 的虚拟地址转换为物理地址，读入 eax 寄存器
    movl    $(V2P_WO(entrypgdir)), %eax     # $(V2P_WO(entrypgdir)) -> %eax 
        # entrypgdir 物理地址写回 %cr3
    movl    %eax, %cr3              # (%eax) -> (%cr3) 

# 开启分页功能
        # %CR0 包含与处理器操作模式和功能相关的标志位
    movl    %cr0, %eax              # (%cr0) -> (%eax)
        # CR0_PG(分页标志),CR0_WP(写保护标志) 置 1 并存入 %eax
    orl     $(CR0_PG|CR0_WP), %eax  # (%eax) = (CR0_PG|CR0_WP) | (%eax) 
        # 开启保护模式、分页功能、写保护功能。
    movl    %eax, %cr0              # (%eax) -> (%cr0)

# 设置栈指针
        # 将栈顶地址(stack + KSTACKSIZE(4096))赋值给栈指针寄存器 esp
    movl $(stack + KSTACKSIZE), %esp    # $(stack + KSTACKSIZE) -> (%esp)

# 跳转到 main()，切换在高地址执行
        # 间接调用是必要的，因为汇编器会为直接跳转生成相对 PC 的指令。
    mov $main, %eax                 # ($main) -> (%eax)
    jmp *%eax                       # jump to main()

# 声明大小为 KSTACKSIZE 的栈供 main() 使用
.comm stack, KSTACKSIZE