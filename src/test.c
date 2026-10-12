#include <stdio.h>

int main()
{
    int a = 1337;
    int b;
    __asm__ volatile (
        "mov %1, %0":
        "=r"(b):
        "r"(a):
    );
    printf("%d\n",b);
}