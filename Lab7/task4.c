#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MAX_LEN 256


typedef struct
{
    int number_deleted;
    char *array_deleted;
    // char *result_string;  
} result;


result delete_up(char *inp_string)
{
    result res;
    size_t len_inp = strlen(inp_string);
    res.number_deleted = 0;
    res.array_deleted = (char*)calloc(len_inp + 1, sizeof(char));
    
    int last_val_deleted = 0;
    int write_idx = 0;

    for (int read_idx = 0; read_idx < (int)len_inp; read_idx++)
    {
        if ((inp_string[read_idx] >= 'A') && (inp_string[read_idx] <= 'Z'))
        {
            char char_to_search[2] = { inp_string[read_idx], '\0' };
            
            if (strcspn(res.array_deleted, char_to_search) == strlen(res.array_deleted))
            {
                res.array_deleted[last_val_deleted] = inp_string[read_idx];
                last_val_deleted++;
            }
            res.number_deleted += 1;
        } 
        else 
        {
            inp_string[write_idx] = inp_string[read_idx];
            write_idx++;
        }
    }
    
    inp_string[write_idx] = '\0';
    
    return res;
}

int main()
{
    char buffer[MAX_LEN];
    printf("Enter the string > ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';

    result (*p_delete_up)(char *inp_string) = delete_up;

    result res = p_delete_up(buffer);

    printf("Number deleted uppercase chars: %3d\nDeleted chars: %s\nResult string: %s\n", 
        res.number_deleted, res.array_deleted, buffer);
}