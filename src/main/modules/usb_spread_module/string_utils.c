#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"

char *findSubstring(char* string, char *substr) {
    char* whereIsIt = global->strstr(string, substr);
    
    return whereIsIt;
}

char *concat(const char *s1, const char *s2) {
    char *result = (char *) global->malloc(global->strlen(s1) + global->strlen(s2) + 1);
    if (result == NULL) {
        DEBUG_LOG_ERR("[USB] concat strings error alloc mem\n");
        return NULL;
    }

    global->strcpy(result, s1);
    global->strcat(result, s2);
    return result;
}