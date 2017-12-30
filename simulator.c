
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
    sim->pc = 0x0000;
    sim->ip = 0xFFFF;
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


void sim_print(Simulator *sim)
{
    printf("PC: 0x%04X\n", sim->pc);
    printf("IP: 0x%04X\n", sim->ip);
}


void sim_disassemble(Simulator *sim, unsigned short addr, int num)
{
    // TODO
    printf("sim_disassemble is not yet implemented!\n");
}

