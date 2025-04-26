#include "asm.h"
#include "memlayout.h"
#include "mmu.h"

# 计算机启动，默认实模式，16-bit 模式
.code16                                 # 告知汇编器生成16-bit模式的代码
.globl start                            # 声明全局符号start

start:                                  # 定义代码起始标签
# 关闭中断
    cli                             
# 清零寄存器 %ax,%ds,%es,%ss
    xorw    %ax,%ax                 
    movw    %ax,%ds                 
    movw    %ax,%es                 
    movw    %ax,%ss                 

# 从实模式切换到保护模式，并且使用引导全局描述符表(GDT)
    lgdt    gdtdesc                     # 从 gdtdesc 读取数据，前 2 字节作为 GDT 的界限，后 4 字节作为 GDT 的基地址，将这些信息加载入 GDTR 
    movl    %cr0, %eax                  # (%cr0) -> (%eax)
    orl     $CR0_PE, %eax               # (%eax) = ($CR0_PE) | (%eax)
    movl    %eax, %cr0                  # (%eax) -> (%cr0)

# 通过使用长跳转指令重新加载代码段寄存器 % cs 和指令指针 % eip，完成向 32-bit 保护模式的转换
    ljmpl    $(SEG_KCODE<<3), $(start32)# reset %cs($(SEG_KCODE<<3)), %eip($start32)

# 切换保护模式， 32-bit 模式
//PAGEBREAK!
.code32
start32:
    movw    $(SEG_KDATA<<3), %ax        # ($(SEG_KDATA<<3)) -> (%ax)
    movw    %ax, %ds                    # (%ax) -> (%ds)
    movw    %ax, %es                    # (%ax) -> (%es)
    movw    %ax, %ss                    # (%ax) -> (%ss)
    movw    $0, %ax                     # ($0) -> (%ax)
    movw    %ax, %fs                    # (%ax) -> (%fs)
    movw    %ax, %gs                    # (%ax) -> (%gs)

# 开启 4MB 页面大小扩展
        # %CR4 包含与处理器功能和操作模式相关的标志位
    movl    %cr4, %eax                  # (%cr4) -> (%eax)
        # CR4_PSE(页面大小扩展标志位)置 1，开启 4MB 页面大小扩展
    orl     $(CR4_PSE), %eax            # (%eax) = ($CR0_PE) | (%eax)
    movl    %eax, %cr4                  # (%eax) -> (%cr4) 

# 使用 entrypgdir 作为初始页表
        # MEM(start - 12)存储 entrypgdir 物理地址
    movl    (start-12), %eax            # MEM(start - 12) -> (%eax)
        # %CR3(页目录基址寄存器)用于存储页目录的物理地址
    movl    %eax, %cr3                  # (%eax) -> (%cr3) 

# 开启分页功能
        # %CR0 包含与处理器操作模式和功能相关的标志位
    movl    %cr0, %eax                  # (%cr0) -> (%eax)
        # CR0_PE(保护模式启用标志位),CR0_PG(分页启用标志位),CR0_WP(写保护标志位) 置 1 并存入 %eax
    orl     $(CR0_PE|CR0_PG|CR0_WP), %eax   # (%eax) = (CR0_PE|CR0_PG|CR0_WP) | (%eax) 
        # 开启保护模式、分页功能、写保护功能。
    movl    %eax, %cr0                  # (%eax) -> (%cr0)

# 切换到由 startothers() 函数分配的栈
        # 栈指针切换到由 startothers() 函数分配的栈上
    movl    (start-4), %esp             # MEM(start - 4) -> (%esp)
                        
# 调用 mpenter() 函数
    call	 *(start-8)                 # call mpenter()

# 处理 bootmain 意外返回
        # ($0x8a00) -> port 0x8a00
    movw    $0x8a00, %ax            
    movw    %ax, %dx                
    outw    %ax, %dx                
        # ($0x8ae0) -> port 0x8a00
    movw    $0x8ae0, %ax            
    outw    %ax, %dx                
spin:
    jmp     spin                        # loop

# 全局描述符表(GDT)定义和相关信息存储
.p2align 2                              # 设置 4-bit 对齐
gdt:
  SEG_NULLASM                           # 定义空段描述符

# SEG_ASM用于定义段描述符
# STA_X - 可执行段  STA_W - 可读段  STA_R - 可写段

  SEG_ASM(STA_X|STA_R, 0, 0xffffffff)   # 定义可读可执行代码段
  SEG_ASM(STA_W, 0, 0xffffffff)         # 定义可写数据段

# 存储 GDT 界限和基地址的结构体
gdtdesc:
  .word   (gdtdesc - gdt - 1)           # 定义界限
  .long   gdt                           # 定义基地址