#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


#define MAX_LEN 256


int stringcmp(char* str1, char* str2)
{
    if (str1[0] < str2[0]) return 1;
    if (str1[0] > str2[0]) return -1;
    return 0;
}

void stringswap(char** str1, char** str2)
{
    char *temp = *str1;
    *str1 = *str2;
    *str2 = temp;
}

void buble_sort(char **text, int number_lines, void (*swap)(char *, char *), int (*cmp)(char*, char*))
{

    int is_changes;
    for (int i = 0; i < number_lines; i++)
    {
        is_changes = 0;
        for (int j = 0; j < number_lines - 1; j++)
        {
            if (cmp(text[j], text[j+1]) == -1)
            {
                is_changes = 1;
                swap(&text[j], &text[j+1]);
            }
        }
        if (is_changes != 1) break;
    }
}

int main()
{
    int number_lines;
    char buffer[MAX_LEN];

    do
    {
        printf("Enter the number of lines of input text (>0) > ");
        scanf("%d", &number_lines);
    } while (number_lines <= 0);
    char **text = (char**)malloc(number_lines * sizeof(char*));
    if (text == NULL)
    {
        perror("malloc");
        exit(1);
    }

    getchar();
    for (int i = 0; i < number_lines; i++)
    {
        printf("Enter the %d line of text (max lengh is 256 chars) >\n", i + 1);
        while (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            buffer[strcspn(buffer, "\n")] = '\0';
            text[i] = (char*)malloc((strlen(buffer) + 1) * sizeof(char));
            strcpy(text[i], buffer);
            break;
        }
    }

    int (*p_stringcmp)(char*, char*) = stringcmp;
    void (*p_stringswap)(char **, char **) = stringswap;
    buble_sort(text, number_lines, p_stringswap, p_stringcmp);

    printf("End sorting of lines based on first char in line.\n");
    for (int i = 0; i < number_lines; i++)
    {
        printf("Line %02d: %s\n", i + 1, text[i]);
        free(text[i]);
    }
    free(text);
    return 0;
}
