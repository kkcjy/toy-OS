#include "types.h"
#include "sleeplock.h"
#include "disk.h"


// buffer of disk to reduce the operation of I/O
struct buffer {
    int flags;              // flag bits
    uint dev;               // identification of disk
    uint blockno;           // identification of block in disk 
    struct Sleeplock lock;  // sleeplock
    uint refcnt;            // counts of citing this buffer

    struct buffer *prev;    // prev + next -> build LRU cache list
    struct buffer *next;    
    struct buffer *qnext;   // point to next buffer in queue
    
    uchar data[BSIZE];      // store data read from disk
};

#define B_VALID 0x2         // buffer has been read from disk
#define B_DIRTY 0x4         // buffer needs to be written to disk