
#include <stdlib.h>
#include <stdio.h>

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
    sim->pc = 0;
    sim->halted = FALSE;

    unsigned short len;
    fread(&len, sizeof(len), 1, file);
    fread(sim->memory, sizeof(char), len, file);

    fclose(file);

    return sim;
}


void sim_load_symbols(Simulator *sim, char *symfile)
{
    printf("sim_load_symbols is not yet implemented!\n");
    // TODO
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

    switch (opcode)
    {
        case OP_NOP:
            break;

        case OP_HLT:
            printf("HLT at 0x%04X\n", loc);
            sim->halted = TRUE;
            sim->pc--;
            break;

        default:
            printf("Illegal opcode 0x%02X at 0x%04X\n", opcode, loc);
            sim->halted = TRUE;
            break;
    }
}


void sim_print(Simulator *sim)
{
    printf("PC: 0x%04X\n", sim->pc);
}


void sim_disassemble(Simulator *sim, unsigned short addr, int num)
{
    // TODO
    printf("sim_disassemble is not yet implemented!\n");
}

