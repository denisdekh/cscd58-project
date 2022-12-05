#include "common.h"

/*
 Helper function to determine if string is empty or not.
 Reads only MAX_LINE characters
*/
int is_empty(char *str)
{
    return str == NULL || strnlen(str, MAX_LINE) == 0;
}