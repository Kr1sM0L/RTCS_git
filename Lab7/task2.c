#include <stdio.h>
#include <stdlib.h>
#include <math.h>


void applyArray(double *float_array, int len, double (*func)(double))
{
    for (int i = 0; i < len; i++)
    {
        float_array[i] = func(float_array[i]);
    }
}

double func(double inp_value)
{
    return (sqrt((inp_value + 0.5) * (inp_value + 0.5) + 0.1) - inp_value * 0.5);
}

int main()
{
    int number_floats;
    do
    {
        printf("Enter the number of floats in input array > ");
        scanf("%d", &number_floats);
    } while (number_floats <= 0);
    getchar();
    
    double *float_array = (double *)malloc(number_floats * sizeof(double));
    for (int i = 0; i < number_floats; i++)
    {
        printf("Enter the %i float > ", i + 1);
        scanf("%lf", &float_array[i]);
        getchar();
    }

    double (*p_func)(double) = func;

    applyArray(float_array, number_floats, p_func);

    printf("End of array conversion.\n|");
    for (int i = 0; i < number_floats; i++)
    {
        printf("%.2lf ", float_array[i]);
    }
    printf("|\n");
    free(float_array);
}