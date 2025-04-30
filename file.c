#include "file.h"
#include "param.h"
#include "spinlock.h"


extern struct Devswitch_t devswitch[NDEV];

struct File_t {
    struct File file[NSFILE];
    struct Spinlock lock;
} filetable;

// init
void fileinit(void) {
  initlock(&filetable.lock, "filetable");
}

// allocate a file
struct File* filealloc(void) {
  struct File *f;
  acquire(&filetable.lock);
  for(f = filetable.file; f < filetable.file + NSFILE; f++){
    if(f->ref == 0){
      f->ref = 1;
      release(&filetable.lock);
      return f;                 // success
    }
  }
  release(&filetable.lock);
  return 0;                     // fail
}

// increment ref count
struct File* filedup(struct File *f) {
    acquire(&filetable.lock);
    if(f->ref < 1)
        panic("filedup");
    f->ref++;
    release(&filetable.lock);
    return f;
}

// close file
void fileclose(struct File *f) {
    struct File ff;

    acquire(&filetable.lock);
    if(f->ref < 1)
        panic("fileclose");
    // close file(remain others)
    if(--f->ref > 0){
        release(&filetable.lock);
        return;
    }
    ff = *f;
    f->ref = 0;
    f->type = FD_NONE;
    release(&filetable.lock);
    // later~~~
    // if(ff.type == FD_PIPE)
    //     pipeclose(ff.pipe, ff.writable);
    // else if(ff.type == FD_INODE){
    //     begin_op();
    //     iput(ff.ip);
    //     end_op();
    // }
}

// Get metadata about file f
int filedata(struct File *f) {
// int filestat(struct File *f, struct stat *st) {
    // later~~~
    // if(f->type == FD_INODE){
    //     ilock(f->ip);
    //     stati(f->ip, st);
    //     iunlock(f->ip);
    //     return 0;
    // }
    // return -1;
}

// read from file
int fileread(struct File *f, char *addr, int n) {
    int r;
    if(!f->readable)            // unreadable
        return -1;
    // if(f->type == FD_PIPE)
    //     return piperead(f->pipe, addr, n);
    // if(f->type == FD_INODE){
    //     ilock(f->ip);
    //     if((r = readi(f->ip, addr, f->off, n)) > 0)
    //     f->off += r;
    //     iunlock(f->ip);
    //     return r;
    // }
    panic("fileread");
}

// write to file
int filewrite(struct File *f, char *addr, int n) {
    int r;

    if(!f->writable)            // unwritable
        return -1;
    // if(f->type == FD_PIPE)
    //     return pipewrite(f->pipe, addr, n);
    // if(f->type == FD_INODE){
    /*
        write a few blocks at a time to avoid exceeding
        the maximum log transaction size
    */
    //     int max = ((MAXOPBLOCKS-1-1-2) / 2) * 512;
    //     int i = 0;
    //     while(i < n){
    //     int n1 = n - i;
    //     if(n1 > max)
    //         n1 = max;

    //     begin_op();
    //     ilock(f->ip);
    //     if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
    //         f->off += r;
    //     iunlock(f->ip);
    //     end_op();

    //     if(r < 0)
    //         break;
    //     if(r != n1)
    //         panic("short filewrite");
    //     i += r;
    //     }
    //     return i == n ? n : -1;
    // }
    panic("filewrite");
}