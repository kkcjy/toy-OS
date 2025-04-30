#include "types.h"
#include "disk.h"
#include "buffer.h"


// File system implementation.  Five layers:
//   + Blocks: allocator for raw disk blocks.
//   + Log: crash recovery for multi-step updates.
//   + Files: inode allocator, reading, writing, metadata.
//   + Directories: inode with special contents (list of other inodes!)
//   + Names: paths like /usr/rtm/xv6/fs.c for convenient naming.

// one superblock per disk device, here only run one device
struct Superblock superblock;


// read from superblock
void readsuperblock(int dev, struct Superblock* sb){
    

}