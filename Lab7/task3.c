#include <stdio.h>
#include <stdlib.h>


int findpos(void *mass, int n, size_t size_elem, int (*func)(void*))
{
    char *ptr = (char*)mass;

    for (int i = 0; i < n; i++)
    {
        void *curr_elem = ptr + (i * size_elem);
        if (func(curr_elem) == 1)
        {
            return i;
        }
    }
    return -1;
}

int is_even(void *elem)
{
    int *curr = (int*)elem;
    if (*curr % 2 == 0) return 1;
    return 0;
}

int is_uppercase(void *elem)
{
    char *curr = (char*)elem;
    return (int)((*curr >= 'A') && (*curr <= 'Z'));
}

int main()
{
    int int_mass[] = {1, 3, 4, 5, 6};
    int (*p_is_even)(void*) = is_even;
    printf("Result int 1 > %d\n", findpos(int_mass, 5, sizeof(int), p_is_even));

    int int_mass_2[] = {1};
    printf("Result int 2 > %d\n", findpos(int_mass_2, 1, sizeof(int), p_is_even));

    char char_mass[] = "AzafsNBD";
    int (*p_is_uppercase)(void* ) = is_uppercase;
    printf("Result int > %d\n", findpos(char_mass, 8, sizeof(char), p_is_uppercase));

    char char_mass_2[] = "adadarttz";
    printf("Result int > %d\n", findpos(char_mass_2, 9, sizeof(char), p_is_uppercase));
}