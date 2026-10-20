#include <stdio.h>

int main() {
    int int_variable = 314;

    void *v_pointer;

    v_pointer = &int_variable;

    printf("Adress of int variable: %p.\n", &int_variable);
    printf("Value of void pointer: %p.\n", v_pointer);

    float float_variable = 3.14;

    v_pointer = &float_variable;

    printf("Adress of float variable: %p.\n", &float_variable);
    printf("Value of void pointer: %p.\n", v_pointer);
}