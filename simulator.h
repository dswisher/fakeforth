#ifndef SIMULATOR_H
#define SIMULATOR_H

#define MEMSIZE 1<<16

typedef struct Simulator
{
    char *memory;

    // Registers and the like
    unsigned short pc;
} Simulator;

Simulator *sim_init(char *objfile);
void sim_load_symbols(Simulator *sim, char *symfile);
void sim_run(Simulator *sim);
void sim_step(Simulator *sim);
void sim_print(Simulator *sim);
void sim_disassemble(Simulator *sim, unsigned short addr, int num);

#endif
