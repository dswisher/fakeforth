
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
    sim->num_symbols = 0;
    sim->symbols = NULL;
    sim->num_breakpoints = 0;
    sim->breakpoints = malloc(sizeof(unsigned short) * MAX_BREAKPOINTS);
    sim->data_stack = NULL;
    sim->return_stack = NULL;
    sim->call_stack = NULL;

    sim_reset(sim);

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

        if (sim->stopped)
        {
            sim->stopped = FALSE;
            return;
        }

        for (int i = 0; i < sim->num_breakpoints; i++)
        {
            if (sim->breakpoints[i] == sim->pc)
            {
                printf("-> BREAK at 0x%04X.\n", sim->pc);
                return;
            }
        }
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

        case REG_A:
            return sim->a;

        case REG_B:
            return sim->b;

        case REG_C:
            return sim->c;

        case REG_D:
            return sim->d;

        case REG_I:
            return sim->i;

        case REG_J:
            return sim->j;

        case REG_M:
            return sim->m;

        case REG_N:
            return sim->n;

        case REG_X:
            return sim->x;

        case REG_Y:
            return sim->y;

        case REG_Z:
            return sim->z;

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

        case REG_A:
            sim->a = value;
            break;

        case REG_B:
            sim->b = value;
            break;

        case REG_C:
            sim->c = value;
            break;

        case REG_D:
            sim->d = value;
            break;

        case REG_I:
            sim->i = value;
            break;

        case REG_J:
            sim->j = value;
            break;

        case REG_M:
            sim->m = value;
            break;

        case REG_N:
            sim->n = value;
            break;

        case REG_X:
            sim->x = value;
            break;

        case REG_Y:
            sim->y = value;
            break;

        case REG_Z:
            sim->z = value;
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


unsigned short pop_call(Simulator *sim)
{
    if (sim->call_stack == NULL)
    {
        printf("Call stack underflow.\n");
        sim->halted = TRUE;
        return 0;
    }

    StackNode *node = sim->call_stack;
    sim->call_stack = node->next;
    unsigned short value = node->value;
    free(node);
    return value;
}


StackNode *push_value(Simulator *sim, unsigned short value, StackNode *next)
{
    StackNode *node = malloc(sizeof(StackNode));
    node->value = value;
    node->next = next;

    return node;
}


StackNode *push_register(Simulator *sim, unsigned char reg, StackNode *next)
{
    return push_value(sim, get_register(sim, reg), next);
}


void sim_write_byte(Simulator *sim, unsigned short addr, unsigned short value)
{
    sim->memory[addr] = value & 0xFF;
}


void sim_write_word(Simulator *sim, unsigned short addr, unsigned short value)
{
    sim->memory[addr] = value >> 8;         // hi byte
    sim->memory[addr + 1] = value & 0xFF;   // lo byte
}


unsigned short sim_read_byte(Simulator *sim, unsigned short addr)
{
    return sim->memory[addr];
}


unsigned short sim_read_word(Simulator *sim, unsigned short addr)
{
    unsigned char hi_byte = sim->memory[addr];
    unsigned char lo_byte = sim->memory[addr + 1];

    return (hi_byte << 8) + lo_byte;
}


unsigned short consume_byte(Simulator *sim)
{
    unsigned char byte = sim->memory[sim->pc++];

    return byte;
}


unsigned short consume_word(Simulator *sim)
{
    unsigned char hi_byte = sim->memory[sim->pc++];
    unsigned char lo_byte = sim->memory[sim->pc++];

    return (hi_byte << 8) + lo_byte;
}


void execute_load(Simulator *sim, unsigned char mode, unsigned char code)
{
    unsigned char reg1;
    unsigned char reg2;

    switch (mode)
    {
        case ADDR_MODE0:    // LOAD a, b
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            // TODO - disallow for byte opcode?
            set_register(sim, reg1, get_register(sim, reg2));
            break;

        case ADDR_MODE1:    // LOAD a, val
            reg1 = sim->memory[sim->pc++];
            if (code == OP_LDB)
            {
                set_register(sim, reg1, consume_byte(sim));
            }
            else
            {
                set_register(sim, reg1, consume_word(sim));
            }
            break;

        case ADDR_MODE2:    // LOAD a, (b)
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            if (code == OP_LDB)
            {
                set_register(sim, reg1, sim_read_byte(sim, get_register(sim, reg2)));
            }
            else
            {
                set_register(sim, reg1, sim_read_word(sim, get_register(sim, reg2)));
            }
            break;

        case ADDR_MODE3:    // LOAD a, (addr)
            reg1 = sim->memory[sim->pc++];
            if (code == OP_LDB)
            {
                set_register(sim, reg1, sim_read_byte(sim, consume_word(sim)));
            }
            else
            {
                set_register(sim, reg1, sim_read_word(sim, consume_word(sim)));
            }
            break;
    }
}


void execute_store(Simulator *sim, unsigned char mode, unsigned char code)
{
    unsigned char reg1;
    unsigned char reg2;

    switch (mode)
    {
        case ADDR_MODE0:    // STORE a, b
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            // TODO - disallow for byte opcode?
            set_register(sim, reg2, get_register(sim, reg1));
            break;

        case ADDR_MODE1:    // STORE a, $N - invalid
            printf("Unhandled STORE address mode: %d\n", mode);
            sim->halted = TRUE;
            break;

        case ADDR_MODE2:    // STORE a, (b)
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            if (code == OP_STB)
            {
                sim_write_byte(sim, get_register(sim, reg2), get_register(sim, reg1));
            }
            else
            {
                sim_write_word(sim, get_register(sim, reg2), get_register(sim, reg1));
            }
            break;

        case ADDR_MODE3:    // STORE a, (addr)
            reg1 = sim->memory[sim->pc++];
            if (code == OP_STB)
            {
                sim_write_byte(sim, consume_byte(sim), get_register(sim, reg1));
            }
            else
            {
                sim_write_word(sim, consume_word(sim), get_register(sim, reg1));
            }
            break;
    }
}


unsigned short mul_operation(unsigned short a, unsigned short b)
{
    return a * b;
}


unsigned short add_operation(unsigned short a, unsigned short b)
{
    return a + b;
}


unsigned short sub_operation(unsigned short a, unsigned short b)
{
    return a - b;
}


void execute_arithmetic(Simulator *sim, unsigned char mode, unsigned short (*operation)(unsigned short a, unsigned short b))
{
    unsigned char reg1;
    unsigned char reg2;
    unsigned short result;

    switch (mode)
    {
        case ADDR_MODE0:    // ADD a, b
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            result = operation(get_register(sim, reg1), get_register(sim, reg2));
            break;

        case ADDR_MODE1:    // ADD a, val 
            reg1 = sim->memory[sim->pc++];
            result = operation(get_register(sim, reg1), consume_word(sim));
            break;

        case ADDR_MODE2:    // ADD a, (b)
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            set_register(sim, reg1, get_register(sim, reg1) + sim_read_word(sim, get_register(sim, reg2)));
            result = operation(get_register(sim, reg1), sim_read_word(sim, get_register(sim, reg2)));
            break;

        case ADDR_MODE3:    // ADD a, (addr)
            reg1 = sim->memory[sim->pc++];
            result = operation(get_register(sim, reg1), sim_read_word(sim, consume_word(sim)));
            break;
    }

    set_register(sim, reg1, result);
}


void do_compare(Simulator *sim, unsigned short val1, unsigned short val2)
{
    unsigned char equal = (val1 == val2) ? FLAG_EQUAL : 0;
    unsigned char gt = (val1 > val2) ? FLAG_GT : 0;
    unsigned char lt = (val1 < val2) ? FLAG_LT : 0;

    sim->flags = equal | gt | lt;
}


void execute_cmp(Simulator *sim, unsigned char mode)
{
    unsigned char reg1;
    unsigned char reg2;

    switch (mode)
    {
        case ADDR_MODE0:    // CMP a, b
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            do_compare(sim, get_register(sim, reg1), get_register(sim, reg2));
            break;

        case ADDR_MODE1:    // CMP a, val 
            reg1 = sim->memory[sim->pc++];
            do_compare(sim, get_register(sim, reg1), consume_word(sim));
            break;

        case ADDR_MODE2:    // CMP a, (b)
            reg1 = sim->memory[sim->pc++];
            reg2 = sim->memory[sim->pc++];
            do_compare(sim, get_register(sim, reg1), sim_read_word(sim, get_register(sim, reg2)));
            break;

        case ADDR_MODE3:    // CMP a, (addr)
            reg1 = sim->memory[sim->pc++];
            do_compare(sim, get_register(sim, reg1), sim_read_word(sim, consume_word(sim)));
            break;
    }
}


bool condition_always(Simulator *sim)
{
    return TRUE;
}


bool condition_equal(Simulator *sim)
{
    return (sim->flags & FLAG_EQUAL) == FLAG_EQUAL;
}


bool condition_not_equal(Simulator *sim)
{
    return (sim->flags & FLAG_EQUAL) == 0;
}


bool condition_gt(Simulator *sim)
{
    return (sim->flags & FLAG_GT) == FLAG_GT;
}


bool condition_lt(Simulator *sim)
{
    return (sim->flags & FLAG_LT) == FLAG_LT;
}


bool condition_ge(Simulator *sim)
{
    return ((sim->flags & FLAG_GT) == FLAG_GT) || ((sim->flags & FLAG_EQUAL) == FLAG_EQUAL);
}


bool condition_le(Simulator *sim)
{
    return ((sim->flags & FLAG_LT) == FLAG_LT) || ((sim->flags & FLAG_EQUAL) == FLAG_EQUAL);
}


void execute_jump(Simulator *sim, unsigned char mode, bool (*condition)(Simulator *sim))
{
    unsigned char reg;
    unsigned short addr;
    unsigned short newpc;

    switch (mode)
    {
        case ADDR_MODE0:    // JMP a
            reg = sim->memory[sim->pc++];
            newpc = get_register(sim, reg);
            break;

        case ADDR_MODE1:    // JMP addr
            newpc = consume_word(sim);
            break;

        case ADDR_MODE2:    // JMP (a)
            reg = sim->memory[sim->pc++];
            newpc = sim_read_word(sim, get_register(sim, reg));
            break;

        case ADDR_MODE3:    // JMP (addr)
            addr = consume_word(sim);
            newpc = sim_read_word(sim, addr);
            break;
    }

    // Only jump if the condition has been met; can't do this up top, need to
    // update the PC by parsing the modes if we don't jump
    if (condition(sim))
    {
        sim->pc = newpc;
    }
}


void print_stack_minion(char *buf, StackNode *node, unsigned short base)
{
    if (node == NULL)
    {
        return;
    }

    print_stack_minion(buf, node->next, base);

    my_itoa((short)node->value, buf, base);

    fputs(buf, stdout);
    fputc(' ', stdout);
}


void print_stack(Simulator *sim, unsigned short base)
{
    char buf[MAXCHAR];
    StackNode *top = sim->data_stack;

    if (top == NULL)
    {
        fputs("[empty]\n", stdout);
        return;
    }

    print_stack_minion(buf, top, base);

    fputs("\n", stdout);
}


void sim_step(Simulator *sim)
{
    if (sim->halted)
    {
        printf("CPU is in halt state.\n");
        return;
    }

    sim->last_pc = sim->pc;

    unsigned char opcode = sim->memory[sim->pc++];
    unsigned char reg;
    unsigned short addr;

    unsigned char code = opcode & ~0x03;
    unsigned char mode = opcode & 0x03;

    switch (code)
    {
        case OP_NOP:
            break;

        case OP_HLT:
            printf("HLT at 0x%04X\n", sim->last_pc);
            sim->halted = TRUE;
            sim->pc--;
            break;

        case OP_JMP:
            execute_jump(sim, mode, condition_always);
            break;

        case OP_JEQ:
            execute_jump(sim, mode, condition_equal);
            break;

        case OP_JNE:
            execute_jump(sim, mode, condition_not_equal);
            break;

        case OP_JGT:
            execute_jump(sim, mode, condition_gt);
            break;

        case OP_JLT:
            execute_jump(sim, mode, condition_lt);
            break;

        case OP_JGE:
            execute_jump(sim, mode, condition_ge);
            break;

        case OP_JLE:
            execute_jump(sim, mode, condition_le);
            break;

        case OP_LDW:
        case OP_LDB:
            execute_load(sim, mode, code);
            break;

        case OP_ADD:
            execute_arithmetic(sim, mode, add_operation);
            break;

        case OP_SUB:
            execute_arithmetic(sim, mode, sub_operation);
            break;

        case OP_MUL:
            execute_arithmetic(sim, mode, mul_operation);
            break;

        case OP_CMP:
            execute_cmp(sim, mode);
            break;

        case OP_STW:
        case OP_STB:
            execute_store(sim, mode, code);
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

        case OP_NEG:
            reg = sim->memory[sim->pc++];
            set_register(sim, reg, - get_register(sim, reg));
            break;

        case OP_GETC:
            reg = sim->memory[sim->pc++];
            set_register(sim, reg, fgetc(stdin));
            break;

        case OP_PUTC:
            reg = sim->memory[sim->pc++];
            fputc(get_register(sim, reg), stdout);
            break;

        case OP_PUTS:
            reg = sim->memory[sim->pc++];
            addr = get_register(sim, reg);
            fputs((char *)(sim->memory + addr), stdout);
            // fputs(&(sim->memory[get_register(sim, reg)]), stdout);
            break;

        case OP_PSTACK:   // TODO - a hack to quickly implement .S
            reg = sim->memory[sim->pc++];
            print_stack(sim, get_register(sim, reg));
            break;

        case OP_CALL:
            addr = consume_word(sim);
            sim->call_stack = push_value(sim, sim->pc, sim->call_stack);
            sim->pc = addr;
            break;

        case OP_RET:
            sim->pc = pop_call(sim);
            break;

        case OP_DCLR:
            while (sim->data_stack != NULL)
            {
                pop_data(sim);
            }
            break;

        case OP_RCLR:
            while (sim->return_stack != NULL)
            {
                pop_return(sim);
            }
            break;

        case OP_BRK:
            printf("BRK at 0x%04X\n", sim->last_pc);
            if (sim->debugging)
            {
                sim->stopped = TRUE;
            }
            else
            {
                sim->halted = TRUE;
            }
            break;

        default:
            printf("Illegal opcode 0x%02X at 0x%04X (code 0x%02X, mode 0x%02X)\n", opcode, sim->last_pc, code, mode);
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

    char *name = op_register_to_name(code);

    if (name != NULL)
    {
        strcat(buf, name);
    }
    else
    {
        strcat(buf, "??");
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
        case OP_JEQ:
        case OP_JNE:
        case OP_JGT:
        case OP_JLT:
        case OP_JGE:
        case OP_JLE:
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

        case OP_LDW:
        case OP_LDB:
        case OP_STW:
        case OP_STB:
        case OP_DPUSH:
        case OP_RPUSH:
        case OP_DPOP:
        case OP_RPOP:
        case OP_INC:
        case OP_DEC:
        case OP_NEG:
        case OP_PSTACK:
        case OP_PUTC:
        case OP_PUTS:
        case OP_ADD:
        case OP_MUL:
        case OP_SUB:
        case OP_CMP:
            strcat(buf, " ");
            disassemble_register(sim, buf, addr);
            break;

        case OP_CALL:
            strcat(buf, " ");
            disassemble_address(sim, buf, addr);
            break;
    }

    // Handle the second argument
    switch (code)
    {
        case OP_LDW:
        case OP_LDB:
        case OP_STW:
        case OP_STB:
        case OP_ADD:
        case OP_MUL:
        case OP_SUB:
        case OP_CMP:
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

    char *bp = "";
    for (int i = 0; i < sim->num_breakpoints; i++)
    {
        if (start == sim->breakpoints[i])
        {
            bp = "*B*";
            break;
        }
    }

    printf(" %-2s %-3s 0x%04X %-12.12s %-8s %s\n", indi, bp, start, buf2, format_bytes(sim, start, end), buf);
}


void sim_disassemble(Simulator *sim, unsigned short addr, int num)
{
    while (num > 0)
    {
        disassemble_one(sim, &addr);
        --num;
    }
}


void sim_toggle_breakpoint(Simulator *sim, unsigned short addr)
{
    // Does it exist already? If so, remove it from the list...
    for (int i = 0; i < sim->num_breakpoints; i++)
    {
        if (sim->breakpoints[i] == addr)
        {
            // Found it - remove it
            for (int j = 0; j < sim->num_breakpoints - 1; j++)
            {
                sim->breakpoints[j] = sim->breakpoints[j + 1];
            }
            sim->num_breakpoints -= 1;
            printf("Breakpoint at 0x%04X cleared.\n", addr);
            return;
        }
    }

    if (sim->num_breakpoints == MAX_BREAKPOINTS - 1)
    {
        printf("-> too many breakpoints!\n");
        return;
    }

    sim->breakpoints[sim->num_breakpoints++] = addr;
    printf("Breakpoint at 0x%04X set.\n", addr);
}


void sim_reset(Simulator *sim)
{
    sim->pc = 0x0000;
    sim->last_pc = sim->pc;
    sim->ip = 0x0000;
    sim->ca = 0x0000;
    sim->a = 0x0000;
    sim->b = 0x0000;
    sim->c = 0x0000;
    sim->d = 0x0000;
    sim->i = 0x0000;
    sim->j = 0x0000;
    sim->m = 0x0000;
    sim->n = 0x0000;
    sim->x = 0x0000;
    sim->y = 0x0000;
    sim->z = 0x0000;
    sim->flags = 0x0000;
    sim->halted = FALSE;
    sim->stopped = FALSE;
    sim->debugging = FALSE;

    while (sim->data_stack != NULL)
    {
        pop_data(sim);
    }

    while (sim->return_stack != NULL)
    {
        pop_return(sim);
    }

    while (sim->call_stack != NULL)
    {
        pop_call(sim);
    }

    // TODO - clear input buffers
}

