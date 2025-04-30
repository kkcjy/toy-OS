#include "types.h"


// ------------------------------------------------ 中断
// 禁用处理器的外部可屏蔽中断
static inline void cli(void) {
    asm volatile("cli");
}

// 启用处理器的外部可屏蔽中断
static inline void sti(void) {
    asm volatile("sti");
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 中断

// ------------------------------------------------ I/O 
// 从指定的 I/O 端口(port)读取 8-bit 数据
static inline uchar inb(ushort port) {
    uchar data;
    asm volatile("in %1,%0" : "=a" (data) : "d" (port));
    return data;
}

// 从指定的 I/O 端口(port)读取 cnt 个 32-bit 数据到指定内存地址(addr)
static inline void insl(int port, void *addr, int cnt) {
    asm volatile("cld; rep insl" :
                "=D" (addr), "=c" (cnt) :
                "d" (port), "0" (addr), "1" (cnt) :
                "memory", "cc");
}

// 向指定的 I/O 端口(port)输出 8-bit 数据(data)
static inline void outb(ushort port, uchar data) {
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

// 向指定的 I/O 端口(port)输出 16-bit 数据(data)
static inline void outw(ushort port, ushort data) {
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

// 向指定的 I/O 端口(port)输出 32-bit 数据(data)
static inline void outsl(int port, const void *addr, int cnt) {
    asm volatile("cld; rep outsl" :
                "=S" (addr), "=c" (cnt) :
                "d" (port), "0" (addr), "1" (cnt) :
                "cc");
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ I/O


// ------------------------------------------------ MEM
// 原子性交换内存地址 addr 处的值与 newval，并返回该内存地址的旧值
static inline uint xchg(volatile uint *addr, uint newval) {
    uint result;
    // The + in "+m" denotes a read-modify-write operand.
    asm volatile("lock; xchgl %0, %1" :
                 "+m" (*addr), "=a" (result) :
                 "1" (newval) :
                 "cc");
    return result;
}
  
// 将 8-bit 的数据(data)重复(cnt)存储到指定的内存区域(addr)
static inline void stosb(void *addr, int data, int cnt) {
    asm volatile("cld; rep stosb" :
                "=D" (addr), "=c" (cnt) :
                "0" (addr), "1" (cnt), "a" (data) :
                "memory", "cc");
}
// 将 32-bit 的数据(data)重复(cnt)存储到指定的内存区域(addr)
static inline void
stosl(void *addr, int data, int cnt)
{
  asm volatile("cld; rep stosl" :
               "=D" (addr), "=c" (cnt) :
               "0" (addr), "1" (cnt), "a" (data) :
               "memory", "cc");
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MEM

// ------------------------------------------------ register
struct Segdesc;
// 将全局描述符表(GDT)的基地址和界限信息存入数组并加载到全局描述符表寄存器(GDTR)中
static inline void lgdt(struct Segdesc *p, int size) {
  volatile ushort pd[3];

  pd[0] = size-1;
  pd[1] = (uint)p;
  pd[2] = (uint)p >> 16;

  asm volatile("lgdt (%0)" : : "r" (pd));
}

struct Gatedesc;
// 将中断描述符表(IDT)的基地址和界限信息存入数组并加载到中断描述符表寄存器(IDTR)中
static inline void lidt(struct Gatedesc *p, int size) {
  volatile ushort pd[3];

  pd[0] = size-1;
  pd[1] = (uint)p;
  pd[2] = (uint)p >> 16;

  asm volatile("lidt (%0)" : : "r" (pd));
}

// 将 16 位选择子(sel)加载到任务寄存器(TR)中，以指定当前任务状态段
static inline void ltr(ushort sel) {
  asm volatile("ltr %0" : : "r" (sel));
}

// 读取标志寄存器(EFLAGS)的值并将其作为 32 位无符号整数返回
static inline uint readeflags(void) {
  uint eflags;
  asm volatile("pushfl; popl %0" : "=r" (eflags));
  return eflags;
}

// 将 16 位值(v)加载到段寄存器(GS)中，可用于设置特定的段地址
static inline void loadgs(ushort v) {
  asm volatile("movw %0, %%gs" : : "r" (v));
}

// 读取控制寄存器 CR2 的值，并将其作为 32 位无符号整数返回
static inline uint rcr2(void) {
    uint val;
    asm volatile("movl %%cr2,%0" : "=r" (val));
    return val;
  }
  
// 将给定的 32 位值 val 加载到控制寄存器 CR3 中
static inline void lcr3(uint val) {
asm volatile("movl %0,%%cr3" : : "r" (val));
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ register



// trapframe built on stack
struct trapframe {
    // // registers as pushed by pusha
    // uint edi;           // 通用寄存器 EDI，常用于字符串操作中的目的索引
    // uint esi;           // 通用寄存器 ESI，常用于字符串操作中的源索引
    // uint ebp;           // 基址指针寄存器 EBP，用于指向当前栈帧的基地址
    // uint oesp;          // 原始栈指针（ESP）被 pusha 压入栈的值，通常不会被使用，可忽略
    // uint ebx;           // 通用寄存器 EBX，可作为通用数据存储和指针使用
    // uint edx;           // 通用寄存器 EDX，常与 EAX 配合进行 64 位运算或作为 I/O 端口操作的端口号
    // uint ecx;           // 通用寄存器 ECX，常作为循环计数器使用
    // uint eax;           // 通用寄存器 EAX，可用于存储函数返回值、进行算术运算等
  
    // // rest of trap frame
    // ushort gs;          // 段寄存器 GS，用于指定一个内存段，常用于多线程环境下的线程本地存储
    // ushort padding1;    // 填充字段，用于保证结构体成员的字节对齐，提高内存访问效率
    // ushort fs;          // 段寄存器 FS，同样用于指定内存段，在某些系统中用于存储线程相关数据
    // ushort padding2;    // 填充字段，保证字节对齐
    ushort es;          // 指定附加的数据段
    ushort padding3;   
    ushort ds;          // 指定程序默认的数据段
    ushort padding4;    
    // uint trapno;        // 陷阱号，标识引发中断或陷阱的具体类型，不同的中断或陷阱有不同的编号
  
    // // below here defined by x86 hardware
    // uint err;           // 错误码，某些中断或陷阱会附带错误码，用于提供更详细的错误信息
    // ``` physical address of instruction = cs*16 + eip ```
    ushort cs;          // 当前正在执行的代码所处的段
    uint eip;           // 将要执行指令的段内地址偏移量
    ushort padding5;    
    uint eflags;        // 标志寄存器 EFLAGS 的值
  
    // below here only when crossing rings, such as from user to kernel
    // ``` physical address of stack = ss*16 + esp ```
    ushort ss;          // 指定当前栈段在内存中的位置
    uint esp;           // 指向堆栈段的栈顶位置
    ushort padding6;    // 填充字段，保证字节对齐
  };