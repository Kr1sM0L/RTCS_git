#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main() {
    int n;
    int *array;

    printf("Enter the len of array > ");
    
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Error: bad input value, n > 0 or must be int!\n");
        return 1;
    }
    
    array = (int*)malloc(n * sizeof(int));

    if (array == NULL) {
        printf("Error: not enough memory for array!\n");
        return 1;
    }

    printf("Array > [");
    for (int i = 0; i < n; i++) {
        array[i] = i;
        if (i != (n - 1)) {
            printf("%d, ", array[i]);
        } else {
            printf("%d]\n", array[i]);
        }
    };
    
    printf("Adress of array: %p\n", array);
    for (int i = 0; i < n; i++) {
        printf("Value: %3d, adress: %p\n", array[i], &array[i]);
    };

    free(array);

    return 0;
}