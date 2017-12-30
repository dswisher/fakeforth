
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
    sim->ip = 0xFFFF;
    sim->halted = FALSE;
    sim->num_symbols = 0;
    sim->symbols = NULL;

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

        case OP_LOAD:
            reg = sim->memory[sim->pc++];
            hi_byte = sim->memory[sim->pc++];
            lo_byte = sim->memory[sim->pc++];
            switch (reg)
            {
                case REG_IP:
                    sim->ip = (hi_byte << 8) + lo_byte;
                    break;

                default:
                    printf("Illegal register 0x%02X at 0x%04X\n", reg, loc);
                    sim->halted = TRUE;
            }
            break;

        default:
            printf("Illegal opcode 0x%02X at 0x%04X\n", opcode, loc);
            sim->halted = TRUE;
            break;
    }
}


char pretty_buff[MAXCHAR];
char *lookup_pretty_symbol_by_address(Simulator *sim, unsigned short addr)
{
    int i;
    *pretty_buff = 0;
    for (i = 0; i < sim->num_symbols; i++)
    {
        if (sim->symbols[i]->location == addr)
        {
            sprintf(pretty_buff, " (%s)", sim->symbols[i]->name);
            break;
        }
    }

    return pretty_buff;
}


void sim_print(Simulator *sim)
{
    printf("PC: 0x%04X%s\n", sim->pc, lookup_pretty_symbol_by_address(sim, sim->pc));
    printf("IP: 0x%04X%s\n", sim->ip, lookup_pretty_symbol_by_address(sim, sim->ip));
}


void sim_disassemble(Simulator *sim, unsigned short addr, int num)
{
    // TODO
    printf("sim_disassemble is not yet implemented!\n");
}

