#include <stdio.h>
#include <unistd.h>
#define MAX_COUNT 10

int main() {
    int prevValue = 1;
    for (int i = 2; i < 20; i++) {
        printf("%d * %d = ", i, prevValue);
        prevValue = prevValue *  i;
        printf("%d\n", prevValue);
    }
    printf("Process %d is over.\n", getpid());
}