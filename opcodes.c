
#include <string.h>

#include "opcodes.h"

typedef struct NameMap
{
    char *name;
    unsigned char code;
} NameMap;

NameMap name_map[] =
{
    { "NOP", OP_NOP },
    { "JMP", OP_JMP },
    { "LOAD", OP_LOAD },
    { "DPUSH", OP_DPUSH },
    { "RPUSH", OP_RPUSH },
    { "HLT", OP_HLT }
};

int num_ops = sizeof(name_map) / sizeof(name_map[0]);


char *op_code_to_name(unsigned char code)
{
    int i;
    for (i = 0; i < num_ops; i++)
    {
        if (name_map[i].code == code)
        {
            return name_map[i].name;
        }
    }

    // Unknown!
    return "???";
}


unsigned char op_name_to_code(char *name)
{
    int i;
    for (i = 0; i < num_ops; i++)
    {
        if (!strcmp(name, name_map[i].name))
        {
            return name_map[i].code;
        }
    }

    // Unknown!
    return 0;
}

