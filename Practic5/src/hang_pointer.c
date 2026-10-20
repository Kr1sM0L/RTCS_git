#include <stdio.h>
#include <stdlib.h>

int main() {
    int *ptr = (int*)calloc(sizeof(int), 8);

    printf("%p\n", ptr);
    printf("%i\n", *ptr);
    free(ptr);

    printf("%p\n", ptr);
    printf("%i\n", *ptr);
}