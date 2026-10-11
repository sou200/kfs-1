#include <stdio.h>

int main()
{
    int a = 1337;
    int b;
    __asm__ volatile (
        "mov %1, %%eax\n\t"
        "mov %%eax, %0\n\t"
        "add $1, %%eax\n\t"
         : "=r"(b)
         : "r" (a)
         : "%eax"
    );
    printf("%d\n",b);
}