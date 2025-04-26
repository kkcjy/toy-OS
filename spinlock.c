#pragma once
#include "spinlock.h"
#include "proc.h"
#include "mmu.h"
#include "x86.h"
#include "memlayout.h"

void init_lock(struct Spinlock *lock, char *name) {
  lock->locked = 0;
  lock->name = name;
  lock->cpu = 0;
}

// disable interrupts and increment the depth of interrupt disabling
void pushcli(void) {
  int eflags;
  eflags = readeflags();
  cli();
  if(get_cpu()->ncli == 0)
    get_cpu()->intena = eflags & FL_IF;
  get_cpu()->ncli += 1;
}

// enable interrupts and reduce the depth of interrupt disabling
void popcli(void) {
  // interrupt should be disabled when call popcli
  if(readeflags()&FL_IF)
    panic("popcli - interruptible");
  // avoid excessive invocation of ncli()
  if(--get_cpu()->ncli < 0)
    panic("popcli");
  // avoid mistakenly restarting the interrupt when the interrupt itself is already disabled
  if(get_cpu()->ncli == 0 && get_cpu()->intena)
    sti();
}

// check whether this CPU is holding the lock
int holding(struct Spinlock *lock) {
  int r;
  pushcli();
  r = lock->locked && (lock->cpu == get_cpu());
  popcli();
  return r;
}

// check whether this CPU is holding the lock
int holding(struct Spinlock *lock) {
  int r;
  pushcli();
  r = lock->locked && (lock->cpu == get_cpu());
  popcli();
  return r;
}

// follow the %ebp chain to record the current call stack in pcs[] 
void getcallerpcs(void *v, uint pcs[]) {
  uint *ebp;
  int i;

 /* typical stack layout in x86 architecture
  | ...                                             (Higher addresses)
  | caller's local variables  
  | parameters
  | return address (EIP)              `EBP + 4` 
  | caller's base address   (EBP)     `EBP`         (Current base address)
  | local variables                   `EBP - 4...`
  | ...                                             (Lower addresses)
*/

  // v is a parameter, -2 points to EBP
  ebp = (uint*)v - 2;
  for(i = 0; i < 10; i++){
    // avoid entering stack's starting position, kernel base address, invalid addresses
    if(ebp == 0 || ebp < (uint*)KERNBASE || ebp == (uint*)0xffffffff)
      break;
    pcs[i] = ebp[1];        // saved %eip
    ebp = (uint*)ebp[0];    // saved %ebp
  }
  for(; i < 10; i++)
    pcs[i] = 0;
}

// acquire the lock
void acquire(struct Spinlock *lock) {
  // disable interrupts to avoid deadlock.
  pushcli(); 

  if(holding(lock))
    panic("acquire");

  // atomic operation, not break unless lock is free
  while(xchg(&lock->locked, 1) != 0);
  
  // 阻止处理器对内存操作进行重排序
  // 确保在该函数调用之前的所有内存操作都在调用之后的所有内存操作之前完成
  __sync_synchronize();

  // record info about lock acquisition for debugging.
  lock->cpu = get_cpu();
  getcallerpcs(&lock, lock->pcs);
}

// release the lock
void release(struct Spinlock *lock) {
  if(!holding(lock))
    panic("release");

  lock->pcs[0] = 0;
  lock->cpu = 0;

  // 阻止处理器对内存操作进行重排序
  // 确保在该函数调用之前的所有内存操作都在调用之后的所有内存操作之前完成
  __sync_synchronize();

  // release the lock and set locked = 0
  // this code can't use a C assignment, since it might not be atomic
  asm volatile("movl $0, %0" : "+m" (lock->locked) : );

  popcli();
}