
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

                case REG_X:
                    sim->x = (hi_byte << 8) + lo_byte;
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


char *sim_lookup_symbol(Simulator *sim, unsigned short addr)
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
            strcat(buf, " IP");
            break;

        case REG_X:
            strcat(buf, " X");
            break;

        default:
            strcat(buf, " ??");
            break;
    }
}


char *format_addr(unsigned short addr)
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


void disassemble_address(Simulator *sim, char *buf, unsigned short *addr)
{
    unsigned char hi_byte = sim->memory[(*addr)++];
    unsigned char lo_byte = sim->memory[(*addr)++];
    unsigned short val = (hi_byte << 8) + lo_byte;
    strcat(buf, " ");
    strcat(buf, format_addr(val));

    char *sym = sim_lookup_symbol(sim, val);
    if (sym != NULL)
    {
        strcat(buf, " (");
        strcat(buf, sym);
        strcat(buf, ")");
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
    switch (code)
    {
        case OP_LOAD:
            disassemble_register(sim, buf, addr);
            disassemble_address(sim, buf, addr);
            break;

        case OP_JMP:
            disassemble_address(sim, buf, addr);
            break;
    }

    unsigned short end = *addr;

    char *sym = sim_lookup_symbol(sim, start);
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

