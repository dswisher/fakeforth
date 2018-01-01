
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "common.h"
#include "simulator.h"

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


struct Context
{
    Simulator *sim;
    StackNode *stack;
    DebugCommand *commands;
    bool stop_requested;
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
        options->infile = strdup(scratch);
    }
    else
    {
        options->infile = argv[1];
    }

    strcpy(scratch, options->infile);
    dot = strrchr(scratch, '.');
    *dot = 0;

    strcat(scratch, ".sym");
    options->symfile = strdup(scratch);

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


bool convert_to_number(char *word, unsigned short *value)
{
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


bool execute_command(Context *context)
{
    char buf[MAXCHAR];
    sprintf(buf, "0x%04X: ", context->sim->pc);
    char *args[MAXARGS];

#ifdef USE_READLINE
    // TODO - fix possible buffer overflow!
    char *line = readline(buf);
    strcpy(buf, line);
    if (strlen(buf) > 0 && strcmp(buf, "q"))
    {
        add_history(buf);
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

    int i;
    for (i = 0; i < num; i++)
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

        // TODO - if it looks like a string, push that

        printf("Command '%s' is not known!\n", word);

        // Keep going
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


void dc_push_x(Context *context)
{
    do_push(context, context->sim->x);
}


void dc_push_y(Context *context)
{
    do_push(context, context->sim->y);
}


void dc_dot(Context *context)
{
    unsigned short value = do_pop(context);
    char *symbol = sim_lookup_symbol(context->sim, value);

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
    unsigned short addr = context->sim->pc;
    if (context->stack != NULL)
    {
        addr = do_pop(context);
    }

    // TODO - add variable to keep track of how many instructions to disassemble
    sim_disassemble(context->sim, addr, 3);
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

    puts("");
    if (sim->halted)
    {
        printf("    *** HALTED ***\n\n");
    }

    format_stack(ds, sim->data_stack);
    format_stack(rs, sim->return_stack);

    printf("    PC: 0x%04X        Data: %s\n", sim->pc, ds);
    printf("    IP: 0x%04X      Return: %s\n", sim->ip, rs);
    printf("    CA: 0x%04X\n", sim->ca);
    printf("     X: 0x%04X\n", sim->x);
    printf("     Y: 0x%04X\n", sim->y);

    puts("");

    sim_disassemble(sim, sim->pc, 3);
}


void dc_syms(Context *context)
{
    Simulator *sim = context->sim;
    int i;
    for (i = 0; i < sim->num_symbols; i++)
    {
        printf("   0x%04X  %s\n", sim->symbols[i]->location, sim->symbols[i]->name);
    }
}


void dc_dump(Context *context)
{
    if (context->stack == NULL)
    {
        printf("You must push an address on the stack, first.\n");
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

    add_command(context, "pc", dc_push_pc);
    add_command(context, "ip", dc_push_ip);
    add_command(context, "ca", dc_push_ca);
    add_command(context, "x", dc_push_x);
    add_command(context, "y", dc_push_y);
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

    // TODO - help
    // TODO - set/clear breakpoint

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

