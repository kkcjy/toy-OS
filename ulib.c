#include "types.h"
#include "x86.h"


// fill the first n bytes of the memory pointed to by dst with byte value c and return dst
void* memset(void *dst, int c, uint n) {
    stosb(dst, c, n);
    return dst;
}