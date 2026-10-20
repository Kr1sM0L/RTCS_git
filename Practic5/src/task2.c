#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main() {
    int n;

    printf("Enter the high and width of two dimension array > ");
    
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Error: bad input value, n > 0 or must be int!\n");
        return 1;
    }
    
    int (*array)[n] = malloc(n * sizeof(*array));

    if (array == NULL) {
        printf("Error: not enough memory for array!\n");
        return 1;
    }

    printf("Array > \n[");
    for (int i = 0; i < n; i++) {
        printf("[");
        for (int k = 0; k < n; k++) {
            array[i][k] = n * i + k;
            if (k != (n - 1)) {
            printf("%d, ", array[i][k]);
            } else {
                printf("%d]", array[i][k]);
            }
        }
        if (i != (n - 1)) {
            printf(",\n");
        } else {
            printf("]\n");
        }
    };
    
    printf("Adress of array: %p\n", array);
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            printf("Value [%2d, %2d]: %3d, adress: %p\n", i, k, array[i][k], &array[i][k]);
        }
    };

    free(array);

    return 0;
}