
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "simulator.h"
#include "opcodes.h"


Simulator *sim_init(char *objfile)
{
    FILE *file = fopen(objfile, "rb");
    if (file == NULL)
    {
        printf("Could not open object file: %s\n", objfile);
        return NULL;
    }

    Simulator *sim = malloc(sizeof(Simulator));
    sim->memory = malloc(MEMSIZE);
    sim->pc = 0x0000;
    sim->ip = 0x0000;
    sim->ca = 0x0000;
    sim->x = 0x0000;
    sim->y = 0x0000;
    sim->halted = FALSE;
    sim->num_symbols = 0;
    sim->symbols = NULL;
    sim->data_stack = NULL;
    sim->return_stack = NULL;

    unsigned short len;
    fread(&len, sizeof(len), 1, file);
    fread(sim->memory, sizeof(char), len, file);

    fclose(file);

    return sim;
}


void sim_load_symbols(Simulator *sim, char *symname)
{
    char str[MAXCHAR];
    FILE *symfile = fopen(symname, "r");
    if (symfile == NULL)
    {
        return;
    }

    int num = atoi(fgets(str, MAXCHAR, symfile));
    sim->symbols = malloc(num * sizeof(SimSymbol));
    while (fgets(str, MAXCHAR, symfile) != NULL)
    {
        str[4] = '\0';
        unsigned short loc = strtol(str, NULL, 16);
        char *sym = str + 5;
        char *pos = strchr(sym, '\n');
        *pos = 0;

        SimSymbol *entry = malloc(sizeof(SimSymbol));
        entry->name = strdup(sym);
        entry->location = loc;

        sim->symbols[sim->num_symbols++] = entry;
    }
    fclose(symfile);
}


void sim_run(Simulator *sim)
{
    while (!sim->halted)
    {
        sim_step(sim);

        // TODO - check to see if we've hit a breakpoint
    }
}


unsigned short get_register(Simulator *sim, unsigned char reg)
{
    switch (reg)
    {
        case REG_PC:
            return sim->pc;

        case REG_IP:
            return sim->ip;

        case REG_CA:
            return sim->ca;

        case REG_X:
            return sim->x;

        case REG_Y:
            return sim->y;

        default:
            printf("Illegal/unhandled register 0x%02X\n", reg);
            sim->halted = TRUE;
            return 0;
    }
}


void set_register(Simulator *sim, unsigned char reg, unsigned short value)
{
    switch (reg)
    {
        case REG_PC:
            sim->pc = value;
            break;

        case REG_IP:
            sim->ip = value;
            break;

        case REG_CA:
            sim->ca = value;
            break;

        case REG_X:
            sim->x = value;
            break;

        case REG_Y:
            sim->y = value;
            break;

        default:
            printf("Illegal/unhandled register 0x%02X\n", reg);
            sim->halted = TRUE;
            break;
    }
}


unsigned short pop_data(Simulator *sim)
{
    if (sim->data_stack == NULL)
    {
        printf("Data stack underflow.\n");
        sim->halted = TRUE;
        return 0;
    }

    StackNode *node = sim->data_stack;
    sim->data_stack = node->next;
    unsigned short value = node->value;
    free(node);
    return value;
}


unsigned short pop_return(Simulator *sim)
{
    if (sim->return_stack == NULL)
    {
        printf("Return stack underflow.\n");
        sim->halted = TRUE;
        return 0;
    }

    StackNode *node = sim->return_stack;
    sim->return_stack = node->next;
    unsigned short value = node->value;
    free(node);
    return value;
}


StackNode *push_register(Simulator *sim, unsigned char reg, StackNode *next)
{
    StackNode *node = malloc(sizeof(StackNode));
    node->value = get_register(sim, reg);
    node->next = next;

    return node;
}


unsigned short read_memory(Simulator *sim, unsigned short addr)
{
    unsigned char hi_byte = sim->memory[addr];
    unsigned char lo_byte = sim->memory[addr + 1];

    return (hi_byte << 8) + lo_byte;
}


unsigned short read_word(Simulator *sim)
{
    unsigned char hi_byte = sim->memory[sim->pc++];
    unsigned char lo_byte = sim->memory[sim->pc++];

    return (hi_byte << 8) + lo_byte;
}


void sim_step(Simulator *sim)
{
    if (sim->halted)
    {
        printf("CPU is in halt state.\n");
        return;
    }

    unsigned short loc = sim->pc;
    unsigned char opcode = sim->memory[sim->pc++];
    unsigned char reg;
    unsigned char reg2;
    unsigned char hi_byte;
    unsigned char lo_byte;

    switch (opcode)
    {
        case OP_NOP:
            break;

        case OP_HLT:
            printf("HLT at 0x%04X\n", loc);
            sim->halted = TRUE;
            sim->pc--;
            break;

        case OP_JMP:
            hi_byte = sim->memory[sim->pc];
            lo_byte = sim->memory[sim->pc + 1];
            sim->pc = (hi_byte << 8) + lo_byte;
            break;

        case OP_GO:
            reg = sim->memory[sim->pc++];
            sim->pc = read_memory(sim, get_register(sim, reg));
            break;

        case OP_LOAD0:
            reg = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            set_register(sim, reg, get_register(sim, reg2));
            break;

        case OP_LOAD1:
            reg = sim->memory[sim->pc++];
            set_register(sim, reg, read_word(sim));
            break;

        case OP_LOAD2:
            reg = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            set_register(sim, reg, read_memory(sim, get_register(sim, reg2)));
            break;

        case OP_LOAD3:
            reg = sim->memory[sim->pc++];
            set_register(sim, reg, read_memory(sim, read_word(sim)));
            break;

        case OP_DPUSH:
            reg = sim->memory[sim->pc++];
            sim->data_stack = push_register(sim, reg, sim->data_stack);
            break;

        case OP_RPUSH:
            reg = sim->memory[sim->pc++];
            sim->return_stack = push_register(sim, reg, sim->return_stack);
            break;

        case OP_DPOP:
            reg = sim->memory[sim->pc++];
            set_register(sim, reg, pop_data(sim));
            break;

        case OP_RPOP:
            reg = sim->memory[sim->pc++];
            set_register(sim, reg, pop_return(sim));
            break;

        case OP_INC:
            reg = sim->memory[sim->pc++];
            set_register(sim, reg, get_register(sim, reg) + 1);
            break;

        case OP_DEC:
            reg = sim->memory[sim->pc++];
            set_register(sim, reg, get_register(sim, reg) - 1);
            break;

        default:
            printf("Illegal opcode 0x%02X at 0x%04X\n", opcode, loc);
            sim->halted = TRUE;
            break;
    }
}


bool sim_lookup_symbol(Simulator *sim, char *name, unsigned short *addr)
{
    int i;
    for (i = 0; i < sim->num_symbols; i++)
    {
        if (!strcmp(sim->symbols[i]->name, name))
        {
            *addr = sim->symbols[i]->location;
            return TRUE;
        }
    }

    return FALSE;
}


char *sim_reverse_lookup_symbol(Simulator *sim, unsigned short addr)
{
    int i;
    for (i = 0; i < sim->num_symbols; i++)
    {
        if (sim->symbols[i]->location == addr)
        {
            return sim->symbols[i]->name;
        }
    }

    return NULL;
}


void disassemble_register(Simulator *sim, char *buf, unsigned short *addr)
{
    unsigned char code = sim->memory[(*addr)++];

    switch (code)
    {
        case REG_IP:
            strcat(buf, "IP");
            break;

        case REG_CA:
            strcat(buf, "CA");
            break;

        case REG_X:
            strcat(buf, "X");
            break;

        case REG_Y:
            strcat(buf, "Y");
            break;

        default:
            strcat(buf, "??");
            break;
    }
}


char *format_word(unsigned short addr)
{
    static char scratch[7];
    sprintf(scratch, "0x%04X", addr);
    return scratch;
}


char *format_byte(unsigned char b)
{
    static char scratch[3];
    sprintf(scratch, "%02X", b);
    return scratch;
}


// In reality, this should be disassemble word, but we assume it's an address to display syms
void disassemble_address(Simulator *sim, char *buf, unsigned short *addr)
{
    unsigned char hi_byte = sim->memory[(*addr)++];
    unsigned char lo_byte = sim->memory[(*addr)++];
    unsigned short val = (hi_byte << 8) + lo_byte;
    strcat(buf, format_word(val));

    char *sym = sim_reverse_lookup_symbol(sim, val);
    if (sym != NULL)
    {
        strcat(buf, " [");
        strcat(buf, sym);
        strcat(buf, "]");
    }
}


char *format_bytes(Simulator *sim, unsigned short start, unsigned short end)
{
    static char scratch[MAXCHAR];
    strcpy(scratch, "");
    while (start < end)
    {
        strcat(scratch, format_byte(sim->memory[start]));
        start += 1;
    }
    return scratch;
}


void disassemble_one(Simulator *sim, unsigned short *addr)
{
    unsigned short start = *addr;
    char buf[MAXCHAR];
    strcpy(buf, "");
    unsigned char code = sim->memory[(*addr)++];
    strcat(buf, op_code_to_name(code));

    // Handle the first argument
    switch (code)
    {
        case OP_LOAD0:
        case OP_LOAD1:
        case OP_LOAD2:
        case OP_LOAD3:
            strcat(buf, " ");
            disassemble_register(sim, buf, addr);
            break;

        case OP_JMP:
            strcat(buf, " ");
            disassemble_address(sim, buf, addr);
            break;

        case OP_GO:
            strcat(buf, " ");
            disassemble_register(sim, buf, addr);
            break;

        case OP_DPUSH:
        case OP_RPUSH:
        case OP_DPOP:
        case OP_RPOP:
        case OP_INC:
        case OP_DEC:
            strcat(buf, " ");
            disassemble_register(sim, buf, addr);
            break;
    }

    // Handle the second argument
    switch (code)
    {
        case OP_LOAD0:
            strcat(buf, ", ");
            disassemble_register(sim, buf, addr);
            break;

        case OP_LOAD1:
            strcat(buf, ", ");
            disassemble_address(sim, buf, addr);
            break;

        case OP_LOAD2:
            strcat(buf, ", (");
            disassemble_register(sim, buf, addr);
            strcat(buf, ")");
            break;

        case OP_LOAD3:
            strcat(buf, ", (");
            disassemble_address(sim, buf, addr);
            strcat(buf, ")");
            break;
    }

    unsigned short end = *addr;

    char *sym = sim_reverse_lookup_symbol(sim, start);
    char buf2[MAXCHAR];
    if (sym == NULL)
    {
        strcpy(buf2, "");
    }
    else
    {
        sprintf(buf2, "%s:", sym);
    }

    char *indi = "";
    if (start == sim->pc)
    {
        indi = "->";
    }

    printf(" %-2s 0x%04X %-12.12s %-8s %s\n", indi, start, buf2, format_bytes(sim, start, end), buf);
}


void sim_disassemble(Simulator *sim, unsigned short addr, int num)
{
    while (num > 0)
    {
        disassemble_one(sim, &addr);
        --num;
    }
}

