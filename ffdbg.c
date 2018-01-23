
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "common.h"
#include "simulator.h"
#include "opcodes.h"
#include "util.h"
#include "forth.h"

#define SEPS " \t\n"
#define HIST_FILE ".ffhist"
#define DUMP_SIZE (5 * 16)



typedef struct Options
{
    char *infile;
    char *symfile;
} Options;


typedef struct Context Context;

typedef struct DebugCommand
{
    char *name;
    void (*func)(Context *context);
    struct DebugCommand *next;
} DebugCommand;


typedef struct StringPool
{
    char **strings;
    int num_strings;
    int size_strings;
} StringPool;


struct Context
{
    Simulator *sim;
    StackNode *stack;
    DebugCommand *commands;
    bool stop_requested;
    StringPool *string_pool;
};


Options *parse_args(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Incorrect number of arguments!\n");
        printf("Usage: %s <infile>\n", argv[0]);
        return NULL;
    }

    Options *options = malloc(sizeof(Options));

    char scratch[MAXCHAR];
    char *dot = strrchr(argv[1], '.');
    if (dot == NULL)
    {
        strcpy(scratch, argv[1]);
        strcat(scratch, ".fo");
        options->infile = my_strdup(scratch);
    }
    else
    {
        options->infile = argv[1];
    }

    strcpy(scratch, options->infile);
    dot = strrchr(scratch, '.');
    *dot = 0;

    strcat(scratch, ".sym");
    options->symfile = my_strdup(scratch);

    return options;
}


DebugCommand *find_command(Context *context, char *name)
{
    DebugCommand *command = context->commands;
    while (command != NULL)
    {
        if (!strcmp(command->name, name))
        {
            return command;
        }

        command = command->next;
    }

    return NULL;
}


StringPool *sp_init()
{
    StringPool *pool = malloc(sizeof(StringPool));
    pool->num_strings = 0;
    pool->size_strings = 20;    // initial size
    pool->strings = malloc(pool->size_strings * sizeof(char *));
    return pool;
}


unsigned short sp_find_or_add(StringPool *pool, char *str)
{
    // NOTE: using 1-based indexing so that 0 indicates not-found
    for (int i = 0; i < pool->num_strings; i++)
    {
        if (!strcmp(pool->strings[i], str))
        {
            return i + 1;
        }
    }

    // String not found; allocate more space if we need it
    if (pool->num_strings >= pool->size_strings)
    {
        pool->size_strings *= 2;
        pool->strings = realloc(pool->strings, pool->size_strings * sizeof(char *));
    }

    // Add the string
    int idx = pool->num_strings++;
    pool->strings[idx] = my_strdup(str);
    return idx + 1;
}


char *sp_get_string(StringPool *pool, unsigned short idx)
{
    if (idx < 1 || idx > pool->num_strings)
    {
        return NULL;
    }

    return pool->strings[idx - 1];
}


bool convert_to_number(char *word, unsigned short *value)
{
    char *c;
    for (c = word; *c != 0; c++)
    {
        if (!isxdigit(*c))
        {
            return FALSE;
        }
    }
    errno = 0;
    *value = strtol(word, NULL, 16);
    return errno == 0;
}


void do_push(Context *context, unsigned short value)
{
    StackNode *node = malloc(sizeof(StackNode));
    node->value = value;
    node->next = context->stack;
    context->stack = node;
}


unsigned short do_pop(Context *context)
{
    if (context->stack == NULL)
    {
        printf("Nothing on the stack!\n");
        return 0;
    }

    StackNode *node = context->stack;
    context->stack = node->next;
    unsigned short value = node->value;
    free(node);
    return value;
}


void do_push_string(Context *context, char *str)
{
    char buf[MAXCHAR];
    strcpy(buf, str + 1);
    char *pos;
    if ((pos = strrchr(buf, '"')) != NULL)
    {
        *pos = 0;
    }

    unsigned short sid = sp_find_or_add(context->string_pool, buf);

    do_push(context, sid);
}


bool execute_command(Context *context)
{
    char buf[MAXCHAR];
    sprintf(buf, "0x%04X: ", context->sim->pc);
    char *args[MAXARGS];

#ifdef USE_READLINE
    static char *last_line = NULL;

    // TODO - fix possible buffer overflow!
    char *line = readline(buf);
    strcpy(buf, line);
    if (strlen(buf) > 0 && strcmp(buf, "q") && (last_line == NULL || strcmp(last_line, buf)))
    {
        add_history(buf);
        if (last_line != NULL)
        {
            free(last_line);
        }
        last_line = my_strdup(buf);
    }
    free(line);
#else
    fputs(buf, stdout);
    if (fgets(buf, MAXCHAR, stdin) == NULL)
    {
        // ignore blank lines
        return TRUE;
    }
#endif

    int num = 0;
    char *token = strtok(buf, SEPS);
    while(token)
    {
        args[num++] = token;
        token = strtok(NULL, SEPS);
    }

    if (num == 0)
    {
        return TRUE;
    }

    for (int i = 0; i < num; i++)
    {
        char *word = args[i];

        DebugCommand *command = find_command(context, word);
        if (command != NULL)
        {
            command->func(context);
            if (context->stop_requested)
            {
                return FALSE;
            }
            continue;
        }

        unsigned short value;
        if (convert_to_number(word, &value))
        {
            do_push(context, value);
            continue;
        }

        if (*word == '"')
        {
            do_push_string(context, word);
            continue;
        }

        printf("Command '%s' is not known!\n", word);

        // Stop processing the current line (but don't exit the interpreter)
        return TRUE;
    }

    return TRUE;
}


void dc_push_pc(Context *context)
{
    do_push(context, context->sim->pc);
}


void dc_push_ip(Context *context)
{
    do_push(context, context->sim->ip);
}


void dc_push_ca(Context *context)
{
    do_push(context, context->sim->ca);
}


void dc_push_i(Context *context)
{
    do_push(context, context->sim->i);
}


void dc_push_j(Context *context)
{
    do_push(context, context->sim->j);
}


void dc_push_m(Context *context)
{
    do_push(context, context->sim->m);
}


void dc_push_n(Context *context)
{
    do_push(context, context->sim->n);
}


void dc_push_x(Context *context)
{
    do_push(context, context->sim->x);
}


void dc_push_y(Context *context)
{
    do_push(context, context->sim->y);
}


void dc_push_z(Context *context)
{
    do_push(context, context->sim->z);
}


void dc_dot(Context *context)
{
    if (context->stack == NULL)
    {
        printf("<empty stack>\n");
        return;
    }
    unsigned short value = do_pop(context);
    char *symbol = sim_reverse_lookup_symbol(context->sim, value);

    if (symbol == NULL)
    {
        printf("0x%04X\n", value);
    }
    else
    {
        printf("0x%04X (%s)\n", value, symbol);
    }
}


void dc_next(Context *context)
{
    sim_step(context->sim);
}


void dc_run(Context *context)
{
    sim_run(context->sim);
}


void dc_list(Context *context)
{
    if ((context->stack == NULL) || (context->stack->next == NULL))
    {
        printf("usage: (addr n --)\n");
        return;
    }

    unsigned short num = do_pop(context);
    unsigned short addr = do_pop(context);

    sim_disassemble(context->sim, addr, num);
}


void dc_quit(Context *context)
{
    context->stop_requested = TRUE;
}


void format_stack(char *buf, StackNode *top)
{
    strcpy(buf, "");
    if (top == NULL)
    {
        strcat(buf, "nil");
        return;
    }

    int num = 0;
    while (top != NULL && num < 5)
    {
        if (num > 0)
        {
            strcat(buf, ", ");
        }

        strcat(buf, format_word(top->value));

        num += 1;
        top = top->next;
    }
}


void dc_print(Context *context)
{
    Simulator *sim = context->sim;
    char ds[MAXCHAR];
    char rs[MAXCHAR];
    char cs[MAXCHAR];

    puts("");
    if (sim->halted)
    {
        printf("    *** HALTED ***\n\n");
    }

    format_stack(ds, sim->data_stack);
    format_stack(rs, sim->return_stack);
    format_stack(cs, sim->call_stack);

    printf("    PC: 0x%04X    I: 0x%04X    A: 0x%04X     Data: %s\n", sim->pc, sim->i, sim->a, ds);
    printf("    IP: 0x%04X    J: 0x%04X    B: 0x%04X   Return: %s\n", sim->ip, sim->j, sim->b, rs);
    printf("    CA: 0x%04X    M: 0x%04X    C: 0x%04X     Call: %s\n", sim->ca, sim->m, sim->c, cs);
    printf("     X: 0x%04X    N: 0x%04X    D: 0x%04X\n", sim->x, sim->n, sim->d);
    printf("     Y: 0x%04X\n", sim->y);
    printf("     Z: 0x%04X    Flags, lt: %d   eq: %d   gt: %d\n", sim->z,
            (sim->flags & FLAG_LT) == FLAG_LT,
            (sim->flags & FLAG_EQUAL) == FLAG_EQUAL,
            (sim->flags & FLAG_GT) == FLAG_GT);

    puts("");

    if (sim->last_pc != sim->pc)
    {
        sim_disassemble(sim, sim->last_pc, 1);
    }

    sim_disassemble(sim, sim->pc, 3);
}


void dc_syms(Context *context)
{
    Simulator *sim = context->sim;
    for (int i = 0; i < sim->num_symbols; i++)
    {
        printf("   0x%04X  %s\n", sim->symbols[i]->location, sim->symbols[i]->name);
    }
}


void dc_dump(Context *context)
{
    if (context->stack == NULL)
    {
        printf("Need an address on the stack.\n");
        return;
    }

    char ascii[MAXCHAR];
    unsigned short addr = do_pop(context);
    unsigned short i;
    unsigned char value;
    for (i = 0; i < DUMP_SIZE; i++)
    {
        if ((i % 16) == 0)
        {
            if (i > 0)
            {
                printf("  |%s|\n", ascii);
            }
            printf("  0x%04X:", addr + i);
            memset(ascii, 0, MAXCHAR);
        }
        else if ((i % 8) == 0)
        {
            printf(" ");
        }

        value = context->sim->memory[addr + i];
        printf(" %02X", value);
        if ((value < ' ') || (value > '~'))
        {
            ascii[i % 16] = '.';
        }
        else
        {
            ascii[i % 16] = value;
        }
    }

    printf("  |%s|\n", ascii);
}


void dc_emit(Context *context)
{
    unsigned short sid = do_pop(context);
    if (sid == 0)
    {
        return;
    }
    char *str = sp_get_string(context->string_pool, sid);
    printf(" -> |%s|\n", str);
}


void dc_lookup(Context *context)
{
    unsigned short sid = do_pop(context);
    if (sid == 0)
    {
        return;
    }
    char *name = sp_get_string(context->string_pool, sid);
    if (name == NULL)
    {
        return;
    }
    unsigned short addr;
    if (sim_lookup_symbol(context->sim, name, &addr))
    {
        do_push(context, addr);
    }
    else
    {
        printf("Symbol '%s' not found.\n", name);
    }
}


void dc_find(Context *context)
{
    char buf[MAXCHAR];
    unsigned short sid = do_pop(context);
    if (sid == 0)
    {
        return;
    }
    char *name = sp_get_string(context->string_pool, sid);
    if (name == NULL)
    {
        return;
    }
    int name_len = strlen(name);

    unsigned short addr;
    if (!sim_lookup_symbol(context->sim, "var_LATEST", &addr))
    {
        printf("Could not find var_LATEST symbol!\n");
        return;
    }

    // Start with the first dictionary entry
    addr = sim_read_word(context->sim, addr);

    // Keep searching until we find something...
    while (addr != 0)
    {
        unsigned char dict_len = context->sim->memory[addr + 2] & F_LENMASK;
        if (dict_len == name_len)
        {
            memset(buf, 0, MAXCHAR);
            for (int i = 0; i < dict_len; i++)
            {
                buf[i] = context->sim->memory[addr + 3 + i];
            }
            
            if (!strcmp(buf, name))
            {
                do_push(context, addr);
                return;
            }
        }

        // No match yet; check the next one
        addr = sim_read_word(context->sim, addr);
    }

    // No match at all!
    printf("Not found.\n");
}


void dc_toggle_breakpoint(Context *context)
{
    unsigned short addr = do_pop(context);
    sim_toggle_breakpoint(context->sim, addr);
}


void dc_reset(Context *context)
{
    sim_reset(context->sim);
    context->sim->debugging = TRUE;
}


char *read_dict_string(Simulator *sim, unsigned short addr)
{
    unsigned char len = sim_read_byte(sim, addr);

    static char buf[MAXCHAR];
    memset(buf, 0, MAXCHAR);
    for (int i = 0; i < len; i++)
    {
        buf[i] = sim->memory[addr + 1 + i];
    }
    return buf;
}


void dc_dict(Context *context)
{
    if (context->stack == NULL)
    {
        printf("usage: (addr --)\n");
        return;
    }

    unsigned short addr = do_pop(context);

    Simulator *sim = context->sim;

    unsigned short prev = sim_read_word(sim, addr);
    unsigned char len = sim_read_byte(sim, addr + 2);
    unsigned short codeAddr = sim_read_word(sim, addr + 3 + len);

    char name[MAXCHAR];
    strcpy(name, read_dict_string(sim, addr + 2));

    char code[MAXCHAR];
    sprintf(code, "0x%04X", codeAddr);
    char *sym = sim_reverse_lookup_symbol(sim, codeAddr);
    if (sym != NULL)
    {
        strcat(code, " ");
        strcat(code, sym);
    }

    char flags[20];
    strcpy(flags, "");
    if (len & F_IMMED)
    {
        strcat(flags, "I");
    }
    if (len & F_HIDDEN)
    {
        strcat(flags, "H");
    }
    len &= 0x1F;

    printf("         +--------+-------+-----------------+----------------------+\n");
    printf(" 0x%04X: | 0x%04X | %2d %2s | %-15.15s | %-20.20s |\n", addr, prev, len, flags, name, code);
    printf("         +--------+-------+-----------------+----------------------+\n");

    unsigned short docolAddr;
    unsigned short exitAddr;
    unsigned short branchAddr;      // TODO - copout - for now, stop on BRANCH
    sim_lookup_symbol(context->sim, "DOCOL", &docolAddr);
    sim_lookup_symbol(context->sim, "EXIT", &exitAddr);
    sim_lookup_symbol(context->sim, "BRANCH", &branchAddr);
    if (codeAddr == docolAddr)
    {
        addr += 3 + len + 2;
        for (int i = 0; i < 10; i++, addr += 2)
        {
            codeAddr = sim_read_word(sim, addr);

            sprintf(code, "0x%04X", codeAddr);
            sym = sim_reverse_lookup_symbol(sim, codeAddr);
            if (sym != NULL)
            {
                strcat(code, " ");
                strcat(code, sym);
            }

            printf("                                    0x%04X: | %-20.20s |\n", addr, code);

            if (codeAddr == exitAddr || codeAddr == branchAddr)
            {
                break;
            }
        }

        printf("                                            +----------------------+\n");
    }
}


void add_command(Context *context, char *name, void (*func)(Context *context))
{
    DebugCommand *command = malloc(sizeof(DebugCommand));
    command->name = name;
    command->next = context->commands;
    command->func = func;
    context->commands = command;
}


Context *create_context(Simulator *sim)
{
    Context *context = malloc(sizeof(Context));
    context->sim = sim;
    context->stack = NULL;
    context->commands = NULL;
    context->stop_requested = FALSE;
    context->string_pool = sp_init();

    add_command(context, "pc", dc_push_pc);
    add_command(context, "ip", dc_push_ip);
    add_command(context, "ca", dc_push_ca);
    add_command(context, "i", dc_push_i);
    add_command(context, "j", dc_push_j);
    add_command(context, "m", dc_push_m);
    add_command(context, "n", dc_push_n);
    add_command(context, "x", dc_push_x);
    add_command(context, "y", dc_push_y);
    add_command(context, "z", dc_push_z);
    add_command(context, ".", dc_dot);
    add_command(context, "quit", dc_quit);
    add_command(context, "q", dc_quit);
    add_command(context, "n", dc_next);
    add_command(context, "next", dc_next);
    add_command(context, "run", dc_run);
    add_command(context, "list", dc_list);
    add_command(context, "l", dc_list);
    add_command(context, "p", dc_print);
    add_command(context, "print", dc_print);
    add_command(context, "syms", dc_syms);
    add_command(context, "dump", dc_dump);
    add_command(context, "hd", dc_dump);
    add_command(context, "dd", dc_dump);
    add_command(context, "emit", dc_emit);
    add_command(context, "lookup", dc_lookup);
    add_command(context, "find", dc_find);
    add_command(context, "break", dc_toggle_breakpoint);
    add_command(context, "b", dc_toggle_breakpoint);
    add_command(context, "reset", dc_reset);
    add_command(context, "dict", dc_dict);

    // TODO - help

    // TODO - make aliases explicitly aliases, rather than defining commands twice

    return context;
}


int main(int argc, char *argv[])
{
    Options *options = parse_args(argc, argv);
    if (options == NULL)
    {
        return 1;
    }

#ifdef USE_READLINE
    rl_read_init_file("~/.inputrc");
    using_history();
    read_history(HIST_FILE);
#endif

    Simulator *sim = sim_init(options->infile);
    sim->debugging = TRUE;
    sim_load_symbols(sim, options->symfile);

    Context *context = create_context(sim);

    while (execute_command(context))
    {
    }

#ifdef USE_READLINE
    write_history(HIST_FILE);
#endif

    return 0;
}

