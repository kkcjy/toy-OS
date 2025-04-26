#include "asm.h"
#include "memlayout.h"
#include "mmu.h"


# 启用首个CPU并切换32-bit保护模式，跳转到C代码执行
# BIOS将此代码从硬盘首个扇区加载到物理地址0x7c00处内存中
# 实模式下开始执行，%cs（代码段寄存器）的值为0，%ip（指令指针寄存器）的值为7c00

# ----------------------------------------------------
# 1. 初始化和关闭中断，为后续操作创建稳定环境。
# 2. 启用 A20 地址线，突破 1MB 内存访问限制。
# 3. 从实模式切换到 32 位保护模式。
# 4. 初始化保护模式下的段寄存器。
# 5. 设置栈指针并调用 C 语言编写的 bootmain 函数。
# 6. 处理 bootmain 函数意外返回的情况。
# 7. 定义全局描述符表（GDT），为保护模式提供内存段管理机制
# ----------------------------------------------------

# b(byte) - 8-bit  w(word) - 16-bit  l(long) - 32-bit
# %ds   数据段寄存器
# %es   附加段寄存器
# %ss   堆栈段寄存器 
# %fs   线程本地存储(TLS)区域
# %gs   CPU 局部存储区域
# %cr0  控制寄存器


# 计算机启动，默认实模式，16-bit 模式
.code16                             # 告知汇编器生成16-bit模式的代码
.globl start                        # 声明全局符号start

start:                              # 定义代码起始标签
# 关闭中断
    cli                             
# 清零寄存器 %ax,%ds,%es,%ss
        # xorw %a %b        %a = (%a)xor(%b)
    xorw    %ax,%ax                 
        # movw %a %b        %b = (%a)
    movw    %ax,%ds                 
    movw    %ax,%es                 
    movw    %ax,%ss                 
# 早期PC的A20地址线被强制拉低，以兼容只支持 1MB 内存的软件
# 此处通过向特定的I/O端口写入和读取数据，来启用A20地址线，以访问超过1MB的内存

# 端口 0x64(键盘控制器的状态和命令端口)
# 读取端口，可以获取键盘控制器的状态信息
# 写入端口，可以让键盘控制器执行相应操作
# 第 2 位被定义为 “输出缓冲区满” 标志位
# 当这一位为 1 时，键盘控制器的输出缓冲区已满，正忙于处理数据，不能接收新的命令
# 当这一位为 0 时，输出缓冲区为空，键盘控制器可以接收新的命令

# 端口 0x60(键盘数据端口)
# 读取端口，可以获取键盘输入的数据
# 写入端口，可以用于向键盘控制器发送控制数据，开启 A20 地址线的命令通过此端口发送

seta20.1:
        # inb $a %b         %b = ($a)
    inb     $0x64,%al                   # ($0x64) -> (%al)
        # testb $a %b       ZF = ($a)&(%b)  
    testb   $0x2,%al                    # ZF = ($0x2) & (%al)
        # jnz target_label  jump to target_label if ZF == 0
    jnz     seta20.1                    # if (ZF == 0), goto seta20.1

        # movb  &a,%b       (%b) = ($a)
    movb    $0xd1,%al                   # ($0xd1) -> (%al)
        # outb  %a,$be      ($b) = (%a)     
    outb    %al,$0x64                   # (%al) -> port 0x64

seta20.2:
        # Wait for not busy
    inb     $0x64,%al                   
    testb   $0x2,%al
    jnz     seta20.2
        # ($0xdf) -> port 0x60
    movb    $0xdf,%al                   
    outb    %al,$0x60

# 从实模式切换到保护模式，并且使用引导全局描述符表(GDT)
# GDT 让虚拟地址直接映射到物理地址，确保在模式切换过程中有效的内存映射不会发生改变

    # GDTR 存储 GDT 的基地址和界限
    # GDT 是保护模式下用于定义内存段的重要数据结构，包含了多个段描述符
    # 描述符定义了一个内存段的起始地址、长度、访问权限等信息
    # gdtdesc 为内存地址，指向包含 GDT 界限和基地址的结构体
    lgdt    gdtdesc                     # 从 gdtdesc 读取数据，前 2 字节作为 GDT 的界限，后 4 字节作为 GDT 的基地址，将这些信息加载入 GDTR 
    movl    %cr0, %eax                  # (%cr0) -> (%eax)
    orl     $CR0_PE, %eax               # (%eax) = ($CR0_PE) | (%eax)
    movl    %eax, %cr0                  # (%eax) -> (%cr0)

# 通过使用长跳转指令重新加载代码段寄存器 % cs 和指令指针 % eip，完成向 32-bit 保护模式的转换
# 段描述符设为不进行地址转换，因此虚拟地址到物理地址的映射仍为直接映射（即虚拟地址与物理地址相同）
    ljmp    $(SEG_KCODE<<3), $(start32) # reset %cs($(SEG_KCODE<<3)), %eip($start32)

# 切换保护模式， 32-bit 模式
//PAGEBREAK!
.code32  
start32:
    movw    $(SEG_KDATA<<3), %ax    # ($(SEG_KDATA<<3)) -> (%ax)
    movw    %ax, %ds                # (%ax) -> (%ds)
    movw    %ax, %es                # (%ax) -> (%es)
    movw    %ax, %ss                # (%ax) -> (%ss)
    movw    $0, %ax                 # ($0) -> (%ax)
    movw    %ax, %fs                # (%ax) -> (%fs)
    movw    %ax, %gs                # (%ax) -> (%gs)

# 设置栈指针并调用 bootmain.c
        # %esp 指向堆栈段的栈顶位置
    movl    $start, %esp            # ($start) -> (%esp)
    call    bootmain                # call bootmain.c

# 处理 bootmain 意外返回
        # ($0x8a00) -> port 0x8a00
    movw    $0x8a00, %ax            
    movw    %ax, %dx                
    outw    %ax, %dx                
        # ($0x8ae0) -> port 0x8a00
    movw    $0x8ae0, %ax            
    outw    %ax, %dx                
spin:
    jmp     spin                    # loop

# 定义 GDT
.p2align 2                                # 内存 4-byte 对齐
gdt:
  SEG_NULLASM                             # 定义空段描述符占位，以防使用未初始化的段描述符
  SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # 定义覆盖整个 32-bit 地址空间可读可执行代码段
  SEG_ASM(STA_W, 0x0, 0xffffffff)         # 定义覆盖整个 32-bit 地址空间可写数据段

gdtdesc:
  .word   (gdtdesc - gdt - 1)             # 前 16-bit 存储 GDT 界限
  .long   gdt                             # 后 32-bit 存储 GDT 基地址