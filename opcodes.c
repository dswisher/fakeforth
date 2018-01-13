
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
    { "JEQ", OP_JEQ },
    { "JNE", OP_JNE },
    { "JGT", OP_JGT },
    { "JLT", OP_JLT },
    { "JGE", OP_JGE },
    { "JLE", OP_JLE },
    { "DPUSH", OP_DPUSH },
    { "RPUSH", OP_RPUSH },
    { "DPOP", OP_DPOP },
    { "RPOP", OP_RPOP },
    { "INC", OP_INC },
    { "DEC", OP_DEC },
    { "GETC", OP_GETC },
    { "PUTC", OP_PUTC },
    { "ADD", OP_ADD },
    { "SUB", OP_SUB },
    { "CALL", OP_CALL },
    { "RET", OP_RET },
    { "CMP", OP_CMP },
    { "LDW", OP_LDW },
    { "LDB", OP_LDB },
    { "STW", OP_STW },
    { "STB", OP_STB },
    { "HLT", OP_HLT }
};

int num_ops = sizeof(name_map) / sizeof(name_map[0]);


char *op_code_to_name(unsigned char code)
{
    for (int i = 0; i < num_ops; i++)
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
    for (int i = 0; i < num_ops; i++)
    {
        if (!strcmp(name, name_map[i].name))
        {
            return name_map[i].code;
        }
    }

    // Unknown!
    return 0;
}

