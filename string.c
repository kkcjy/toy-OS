#include "types.h"
#include "x86.h"


// 从 dst 开始的连续 n-byte 内存区域填充为字符 c
void* memset(void *dst, int c, uint n) {
    if ((int)dst%4 == 0 && n%4 == 0){
        c &= 0xFF;
        stosl(dst, (c<<24)|(c<<16)|(c<<8)|c, n/4);
    } else
        stosb(dst, c, n);
    return dst;
}

// 比较内存区域内储存数据大小
int memcmp(const void *v1, const void *v2, uint n) {
    const uchar *s1, *s2;
    s1 = v1;
    s2 = v2;
    while(n-- > 0){
        if(*s1 != *s2)
        return *s1 - *s2;
        s1++, s2++;
    }
    return 0;
}

// 从 src 开始 n-bit 数据复制到 dst 指向的内存区域，返回 dst 指针
// 当 src 和 dst 内存区域重叠且 src < dst 时，从后往前复制避免数据覆盖问题
void* memmove(void *dst, const void *src, uint n) {
    const char *s;
    char *d;

    s = src;
    d = dst;
    if(s < d && s + n > d){
        s += n;
        d += n;
        while(n-- > 0)
        *--d = *--s;
    } else
        while(n-- > 0)
        *d++ = *s++;

    return dst;
}

// memcpy exists to placate GCC.  Use memmove.
void* memcpy(void *dst, const void *src, uint n) {
    return memmove(dst, src, n);  
}

// 获取字符串 s 长度
int strlen(const char *s) {
    int n;

    for(n = 0; s[n]; n++)
        ;
    return n;
}

// 比较字符串 p 和 q 的前 n 个字符大小
int strncmp(const char *p, const char *q, uint n) {
    while(n > 0 && *p && *p == *q)
        n--, p++, q++;
    if(n == 0)
        return 0;
    return (uchar)*p - (uchar)*q;
}

// 将字符串 t 的前 n 个字符复制到字符串 s 中，不够补 0
char* strncpy(char *s, const char *t, int n) {
    char *os;
    os = s;
    while(n-- > 0 && (*s++ = *t++) != 0)
        ;
    while(n-- > 0)
        *s++ = 0;
    return os;
}

// 将字符串 t 的前 n 个字符复制到字符串 s 中，不进行补 0
char* safestrcpy(char *s, const char *t, int n) {
    char *os;
    os = s;
    if(n <= 0)
      return os;
    while(--n > 0 && (*s++ = *t++) != 0)
      ;
    *s = 0;
    return os;
}