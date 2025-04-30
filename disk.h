#include "types.h"


// disk file system

#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// Disk layout:
// [ boot block | super block | log | inode blocks | free bit map | data blocks]
/*
boot block：    包含 BIOS 导入内存，用于启动操作系统的代码
super block：   存储文件系统的全局信息，用于文件系统的管理和操作
log：           用于实现日志式文件系统，记录对文件系统的更改操作
inode blocks：  存放文件和目录在文件系统中的对应索引节点，包含文件和目录的元数据
free bit map：  记录磁盘上空闲数据块位置，每个位代表一个数据块，0 空闲，1 已用
data blocks：   存储文件和目录数据区
*/

struct Diskinode;

#define NDIRECT 12          // first NDIRECT blocks stored in inode
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// number of inodes per block
#define IPB             (BSIZE / sizeof(struct Dinode))

// block which contains inode i
#define IBLOCK(i, sb)   ((i) / IPB + sb.inodestart)

// bitmap bits per block
#define BPB             (BSIZE*8)

// block of free map containing bit for block b
#define BBLOCK(b, sb)   (b/BPB + sb.bmapstart)

// max length of file name
#define DIRSIZ          14

// info of disk
struct Superblock {
    uint size;              // size of file system image
    uint nblocks;           // number of data blocks
    uint ninodes;           // number of inodes.
    uint nlog;              // number of log blocks
    uint logstart;          // block number of first log block
    uint inodestart;        // block number of first inode block
    uint bmapstart;         // block number of first free map block
};

// for identify and manage files
struct Diskinode {
    short type;             // file type
    short major;            // major device number (T_DEV only)
    short minor;            // minor device number (T_DEV only)
    short nlink;            // number of links to inode in file system
    uint size;              // size of file (bytes)
    /*
        first NDIRECT elements - direct blocks 
        last element - indirect block
    */
    uint addrs[NDIRECT+1];  // data block addresses
};

// directory entry(directory contains many directory entries)
struct Direntry {
    ushort inum;
    char name[DIRSIZ];
};