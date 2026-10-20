#include <stdio.h>
#include <stdbool.h>
#include <math.h>


typedef struct {
    double x;
    double y;
    double z;
} Vector_3d;

typedef struct {
    double mixed_product;
    double volume_paral;
    double volume_piramid;
} Result;


Result pointer_function(Vector_3d v1, Vector_3d v2, Vector_3d v3) {
    Result result;

    result.mixed_product = v1.x * (v2.y * v3.z - v3.y * v2.z) - 
                           v2.x * (v1.y * v3.z - v3.y * v1.z) +
                           v3.x * (v1.y * v2.z - v2.y * v1.z);
    
    result.volume_paral = fabs(result.mixed_product);

    result.volume_piramid = result.volume_paral / 6.0;

    return result;
}

double check_value(char name) {
    float result;
    while (true) {
        printf("Enter %c > ", name);
        if (scanf("%f", &result) != 1) {
            printf("Error: invalid input value, type must double!\n");
            while (getchar() != '\n');
        } else {
            break;
        }
    }
    return result;
}

void get_vector(Vector_3d *v, char number) {
    printf("Process input value vector%c\n", number);
    v->x = check_value('x');
    v->y = check_value('y');
    v->z = check_value('z');
}


int main() {
    Vector_3d v1;
    Vector_3d v2;
    Vector_3d v3;

    get_vector(&v1, '1');
    get_vector(&v2, '2');
    get_vector(&v3, '3');

    Result (*func_pointer)(Vector_3d, Vector_3d, Vector_3d) = NULL;

    func_pointer = pointer_function;

    Result result = func_pointer(v1, v2, v3);

    printf("Mixed product: %f\n", result.mixed_product);
    printf("Volume parallelepiped: %f\n", result.volume_paral);
    printf("Volume piramid: %f\n", result.volume_piramid);
}