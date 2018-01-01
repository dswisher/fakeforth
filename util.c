
#include <stdlib.h>
#include <string.h>

#include "util.h"

char *my_strdup(char *c)
{
    int len = strlen(c);
    char *buf = malloc(len + 1);
    strcpy(buf, c);
    return buf;
}

