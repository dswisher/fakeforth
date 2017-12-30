
#include <stdlib.h>
#include <stdio.h>

#include "simulator.h"

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
    printf("sim_run is not yet implemented!\n");
    // TODO
}


void sim_step(Simulator *sim)
{
    printf("sim_step is not yet implemented!\n");
    // TODO
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

