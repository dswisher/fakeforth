#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "common.h"

#define MEMSIZE 1<<16
#define MAX_BREAKPOINTS 100

typedef struct SimSymbol
{
    char *name;
    unsigned short location;
} SimSymbol;


typedef struct StackNode
{
    unsigned short value;
    struct StackNode *next;
} StackNode;


typedef struct Simulator
{
    unsigned char *memory;

    // Registers
    unsigned short pc;      // program counter
    unsigned short ip;      // instruction pointer
    unsigned short ca;      // code address pointer
    unsigned short i;       // scratch registers
    unsigned short j;
    unsigned short m;
    unsigned short n;
    unsigned short x;
    unsigned short y;
    unsigned short z;
    unsigned short flags;   // flags register (bits) - uses FLAG_xx macros in opcodes.h

    // Stacks
    StackNode *data_stack;
    StackNode *return_stack;
    StackNode *call_stack;

    // Simulation state
    bool halted;

    // Set of addresses that have breakpoints set
    int num_breakpoints;
    unsigned short *breakpoints;

    // Debugging helpers
    unsigned short last_pc;

    // Symbols
    int num_symbols;
    SimSymbol **symbols;
} Simulator;


Simulator *sim_init(char *objfile);
void sim_load_symbols(Simulator *sim, char *symfile);
void sim_run(Simulator *sim);
void sim_step(Simulator *sim);
void sim_disassemble(Simulator *sim, unsigned short addr, int num);
char *sim_reverse_lookup_symbol(Simulator *sim, unsigned short addr);
bool sim_lookup_symbol(Simulator *sim, char *name, unsigned short *addr);
unsigned short sim_read_word(Simulator *sim, unsigned short addr);
void sim_toggle_breakpoint(Simulator *sim, unsigned short addr);
void sim_reset(Simulator *sim);

char *format_word(unsigned short addr);

#endif
