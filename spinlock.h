#pragma once
#include "types.h"

// Mutual exclusion lock
struct Spinlock {
    uint locked;        // if lock held

    // for Debug
    char *name;         // name of lock
    struct CPU *cpu;    // the cpu holding the lock.
    uint pcs[10];       // the call stack (an array of program counters) that locked the lock
};

void init_lock(struct Spinlock *lock, char *name);
void pushcli(void);
void popcli(void);
int holding(struct Spinlock *lock);
int holding(struct Spinlock *lock);
void getcallerpcs(void *v, uint pcs[]);
void acquire(struct Spinlock *lock);
void release(struct Spinlock *lock);