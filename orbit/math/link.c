extern void quixcc_callme();

#include <stdio.h>

int main()
{
    printf("Calling to QUIX library function...\n");
    quixcc_callme();
    printf("QUIX library function called.\n");

    return 0;
}