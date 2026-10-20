#include <stdio.h>

int main() {
    int variable = 100000;

    int *prt_var = &variable;

    printf("Value of variable: %d.\n", variable);

    printf("Adress of variable: %p.\n", &variable);

    printf("Value of pointer: %p.\n", prt_var);

    printf("Adress of pointer: %p.\n", &prt_var);
}