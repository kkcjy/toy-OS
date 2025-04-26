// 输出、可重定位与可执行文件格式：32位x86架构ELF
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
// 链接器输出文件架构：Intel 80386及兼容32位x86
OUTPUT_ARCH(i386)
// 程序入口点设置为_start
ENTRY(_start)

SECTIONS
{
    // 内核链接虚拟地址，需与KERNLINK一致
    . = 0x80100000;

    // 代码段，物理加载地址0x100000，合并输入文件代码段
    .text : AT(0x100000) {
        *(._text .stub .text.* .gnu.linkonce.t.*)
    }
    // 定义etext为代码段结束地址
    PROVIDE(etext = .);

    // 只读数据段，合并输入文件只读数据段
    .rodata : {
        *(._rodata .rodata.* .gnu.linkonce.r.*)
    }

    // 调试信息段，定义起始和结束符号
    .stab : {
        PROVIDE(__STAB_BEGIN__ = .);
        *(._stab);
        PROVIDE(__STAB_END__ = .);
    }
    .stabstr : {
        PROVIDE(__STABSTR_BEGIN__ = .);
        *(._stabstr);
        PROVIDE(__STABSTR_END__ = .);
    }

    // 地址对齐到4KB边界，确保数据段页起始
    . = ALIGN(0x1000);
    // 定义data为数据段起始地址
    PROVIDE(data = .);

    // 数据段，合并输入文件数据段
    .data : {
        *(._data)
    }
    // 定义edata为数据段结束地址
    PROVIDE(edata = .);

    // 未初始化数据段，合并输入文件对应段
    .bss : {
        *(._bss)
    }
    
    // 定义end为内核映像结束地址
    PROVIDE(end = .);

    // 丢弃输入文件的.eh_frame和.note.GNU-stack段
    /DISCARD/ : {
        *(._eh_frame .note.GNU-stack)
    }
}