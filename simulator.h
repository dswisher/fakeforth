#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "common.h"

#define MEMSIZE 1<<16

typedef struct SimSymbol
{
    char *name;
    unsigned short location;
} SimSymbol;


typedef struct Simulator
{
    char *memory;

    // Registers and the like
    unsigned short pc;      // program counter
    unsigned short ip;      // instruction pointer
    unsigned short x;       // scratch register

    // Simulation state
    bool halted;

    // Symbols
    int num_symbols;
    SimSymbol **symbols;
} Simulator;

Simulator *sim_init(char *objfile);
void sim_load_symbols(Simulator *sim, char *symfile);
void sim_run(Simulator *sim);
void sim_step(Simulator *sim);
void sim_disassemble(Simulator *sim, unsigned short addr, int num);
char *sim_lookup_symbol(Simulator *sim, unsigned short addr);

#endif
