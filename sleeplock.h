#include "types.h"
#include "spinlock.h"


/*
allows threads to enter sleep state while waiting for lock to be released
avoiding busy waiting and improving system efficiency
*/


// Long-term locks for processes
struct Sleeplock {
    uint locked;            // if lock held
    struct Spinlock lock;   // spinlock protecting this sleep lock

    // for Debug
    char *name;             // name of lock
    int pid;                // process holding lock
  };
  
  