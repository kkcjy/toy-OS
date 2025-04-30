#include "types.h"
#include "disk.h"
#include "sleeplock.h"


struct File {
    enum { FD_NONE, FD_PIPE, FD_INODE } type;
    int ref;                // reference count
    char readable;              
    char writable;          
    struct Pipe *pipe;      // for FD_PIPE
    struct Inode *ip;       // for FD_INODE
    uint off;               
};

// in-memory copy of Diskinode in disk to identify files
struct Inode {
    uint dev;               // sign of device
    uint inum;              // sign of inode
    int ref;                // reference count
    struct Sleeplock lock;  // lock
    int valid;              // if valid?
    struct Diskinode cpyinode; // copy of Diskinode
};
  
// device switch table(maps major device number to device read and write functions)
struct Devswitch_t {
    int (*read)(struct inode*, char*, int);     // Pointer to the device read function
    int (*write)(struct inode*, char*, int);    // Pointer to the device write function
};
  
extern struct Devswitch_t devswitch[];          // store different read and write functions

#define CONSOLE 1                               // major device number of the console device