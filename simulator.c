
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "simulator.h"
#include "opcodes.h"
#include "util.h"


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
        entry->name = my_strdup(sym);
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


void sim_write_word(Simulator *sim, unsigned short addr, unsigned short value)
{
    sim->memory[addr] = value >> 8;         // hi byte
    sim->memory[addr + 1] = value & 0xFF;   // lo byte
}


unsigned short sim_read_word(Simulator *sim, unsigned short addr)
{
    unsigned char hi_byte = sim->memory[addr];
    unsigned char lo_byte = sim->memory[addr + 1];

    return (hi_byte << 8) + lo_byte;
}


unsigned short consume_word(Simulator *sim)
{
    unsigned char hi_byte = sim->memory[sim->pc++];
    unsigned char lo_byte = sim->memory[sim->pc++];

    return (hi_byte << 8) + lo_byte;
}


void execute_load(Simulator *sim, unsigned char mode)
{
    unsigned char reg1;
    unsigned char reg2;

    switch (mode)
    {
        case ADDR_MODE0:    // LOAD a, b
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            set_register(sim, reg1, get_register(sim, reg2));
            break;

        case ADDR_MODE1:    // LOAD a, val
            reg1 = sim->memory[sim->pc++];
            set_register(sim, reg1, consume_word(sim));
            break;

        case ADDR_MODE2:    // LOAD a, (b)
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            set_register(sim, reg1, sim_read_word(sim, get_register(sim, reg2)));
            break;

        case ADDR_MODE3:    // LOAD a, (addr)
            reg1 = sim->memory[sim->pc++];
            set_register(sim, reg1, sim_read_word(sim, consume_word(sim)));
            break;
    }
}


void execute_store(Simulator *sim, unsigned char mode)
{
    unsigned char reg1;
    unsigned char reg2;

    switch (mode)
    {
        case ADDR_MODE0:    // STORE a, b
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            set_register(sim, reg2, get_register(sim, reg1));
            break;

        case ADDR_MODE1:    // STORE a, $N - invalid
            printf("Unhandled STORE address mode: %d\n", mode);
            sim->halted = TRUE;
            break;

        case ADDR_MODE2:    // STORE a, (b)
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            sim_write_word(sim, get_register(sim, reg2), get_register(sim, reg1));
            break;

        case ADDR_MODE3:    // STORE a, (addr)
            reg1 = sim->memory[sim->pc++];
            sim_write_word(sim, consume_word(sim), get_register(sim, reg1));
            break;
    }
}


void execute_add(Simulator *sim, unsigned char mode)
{
    unsigned char reg1;
    unsigned char reg2;

    switch (mode)
    {
        case ADDR_MODE0:    // ADD a, b
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            set_register(sim, reg1, get_register(sim, reg1) + get_register(sim, reg2));
            break;

        case ADDR_MODE1:    // ADD a, val 
            reg1 = sim->memory[sim->pc++];
            set_register(sim, reg1, get_register(sim, reg1) + consume_word(sim));
            break;

        case ADDR_MODE2:    // ADD a, (b)
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            set_register(sim, reg1, get_register(sim, reg1) + sim_read_word(sim, get_register(sim, reg2)));
            break;

        case ADDR_MODE3:    // ADD a, (addr)
            reg1 = sim->memory[sim->pc++];
            set_register(sim, reg1, get_register(sim, reg1) + sim_read_word(sim, consume_word(sim)));
            break;
    }
}


void execute_jump(Simulator *sim, unsigned char mode)
{
    unsigned char reg;
    unsigned short addr;

    switch (mode)
    {
        case ADDR_MODE0:    // JMP a
            reg = sim->memory[sim->pc++];
            sim->pc = get_register(sim, reg);
            break;

        case ADDR_MODE1:    // JMP addr
            sim->pc = consume_word(sim);
            break;

        case ADDR_MODE2:    // JMP (a)
            reg = sim->memory[sim->pc++];
            sim->pc = sim_read_word(sim, get_register(sim, reg));
            break;

        case ADDR_MODE3:    // JMP (addr)
            addr = consume_word(sim);
            sim->pc = sim_read_word(sim, addr);
            break;
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

    unsigned char code = opcode & ~0x03;
    unsigned char mode = opcode & 0x03;

    switch (code)
    {
        case OP_NOP:
            break;

        case OP_HLT:
            printf("HLT at 0x%04X\n", loc);
            sim->halted = TRUE;
            sim->pc--;
            break;

        case OP_JMP:
            execute_jump(sim, mode);
            break;

        case OP_LOAD:
            execute_load(sim, mode);
            break;

        case OP_ADD:
            execute_add(sim, mode);
            break;

        case OP_STORE:
            execute_store(sim, mode);
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

        case OP_GETC:
            reg = sim->memory[sim->pc++];
            set_register(sim, reg, fgetc(stdin));
            break;

        case OP_PUTC:
            reg = sim->memory[sim->pc++];
            fputc(get_register(sim, reg), stdout);
            break;

        default:
            printf("Illegal opcode 0x%02X at 0x%04X (code 0x%02X, mode 0x%02X)\n", opcode, loc, code, mode);
            sim->halted = TRUE;
            break;
    }
}


bool sim_lookup_symbol(Simulator *sim, char *name, unsigned short *addr)
{
    for (int i = 0; i < sim->num_symbols; i++)
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
    for (int i = 0; i < sim->num_symbols; i++)
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
    unsigned char opcode = sim->memory[(*addr)++];

    unsigned char code = opcode & ~0x03;
    unsigned char mode = opcode & 0x03;

    strcat(buf, op_code_to_name(code));

    // Handle the first argument
    switch (code)
    {
        case OP_JMP:
            switch (mode)
            {
                case ADDR_MODE0:
                    strcat(buf, " ");
                    disassemble_register(sim, buf, addr);
                    break;

                case ADDR_MODE1:
                    strcat(buf, " ");
                    disassemble_address(sim, buf, addr);
                    break;

                case ADDR_MODE2:
                    strcat(buf, " (");
                    disassemble_register(sim, buf, addr);
                    strcat(buf, ")");
                    break;

                case ADDR_MODE3:
                    strcat(buf, " (");
                    disassemble_address(sim, buf, addr);
                    strcat(buf, ")");
                    break;
            }
            break;

        case OP_LOAD:
        case OP_STORE:
        case OP_DPUSH:
        case OP_RPUSH:
        case OP_DPOP:
        case OP_RPOP:
        case OP_INC:
        case OP_DEC:
        case OP_PUTC:
        case OP_ADD:
            strcat(buf, " ");
            disassemble_register(sim, buf, addr);
            break;
    }

    // Handle the second argument
    switch (code)
    {
        case OP_LOAD:
        case OP_STORE:
        case OP_ADD:
            switch (mode)
            {
                case ADDR_MODE0:
                    strcat(buf, ", ");
                    disassemble_register(sim, buf, addr);
                    break;

                case ADDR_MODE1:
                    strcat(buf, ", ");
                    disassemble_address(sim, buf, addr);
                    break;

                case ADDR_MODE2:
                    strcat(buf, ", (");
                    disassemble_register(sim, buf, addr);
                    strcat(buf, ")");
                    break;

                case ADDR_MODE3:
                    strcat(buf, ", (");
                    disassemble_address(sim, buf, addr);
                    strcat(buf, ")");
                    break;
            }
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

