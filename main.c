#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "defs.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

// bootstrap processor starts running C code here
int main(void) {
    print("hello");
}