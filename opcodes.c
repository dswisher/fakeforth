
#include <string.h>

#include "common.h"
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
    { "NEG", OP_NEG },
    { "GETC", OP_GETC },
    { "PUTC", OP_PUTC },
    { "PUTN", OP_PUTN },
    { "PUTS", OP_PUTS },
    { "PSTACK", OP_PSTACK },
    { "PRSTACK", OP_PRSTACK },
    { "ADD", OP_ADD },
    { "AND", OP_AND },
    { "NOT", OP_NOT },
    { "OR", OP_OR },
    { "XOR", OP_XOR },
    { "MUL", OP_MUL },
    { "SUB", OP_SUB },
    { "CALL", OP_CALL },
    { "RET", OP_RET },
    { "DCLR", OP_DCLR },
    { "RCLR", OP_RCLR },
    { "CMP", OP_CMP },
    { "LDW", OP_LDW },
    { "LDB", OP_LDB },
    { "STW", OP_STW },
    { "STB", OP_STB },
    { "BRK", OP_BRK },
    { "HLT", OP_HLT }
};

int num_ops = sizeof(name_map) / sizeof(name_map[0]);


typedef struct RegisterMap
{
    unsigned char val;
    char *name;
} RegisterMap;

RegisterMap register_map[] =
{
    { REG_IP, "IP" },
    { REG_CA, "CA" },
    { REG_A, "A" },
    { REG_B, "B" },
    { REG_C, "C" },
    { REG_D, "D" },
    { REG_I, "I" },
    { REG_J, "J" },
    { REG_M, "M" },
    { REG_N, "N" },
    { REG_X, "X" },
    { REG_Y, "Y" },
    { REG_Z, "Z" },
};

int num_registers = sizeof(register_map) / sizeof(register_map[0]);


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


bool op_is_register(char *name)
{
    for (int i = 0; i < num_registers; i++)
    {
        if (!strcmp(name, register_map[i].name))
        {
            return TRUE;
        }
    }

    return FALSE;
}


unsigned char op_name_to_register(char *name)
{
    for (int i = 0; i < num_registers; i++)
    {
        if (!strcmp(name, register_map[i].name))
        {
            return register_map[i].val;
        }
    }

    return 0;
}


char *op_register_to_name(unsigned char code)
{
    for (int i = 0; i < num_registers; i++)
    {
        if (code == register_map[i].val)
        {
            return register_map[i].name;
        }
    }

    return NULL;
}

