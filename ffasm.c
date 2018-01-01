
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"
#include "simulator.h"
#include "opcodes.h"


#define SEPS " \t,"


typedef struct ArgCount
{
    unsigned char opcode;
    int num_args;
} ArgCount;

ArgCount arg_counts[] =
{
    { OP_JMP, 1 },
    { OP_LOAD0, 2 },
    { OP_LOAD1, 2 },
    { OP_LOAD2, 2 },
    { OP_LOAD3, 2 },
    { OP_DPUSH, 1 },
    { OP_RPUSH, 1 },
    { OP_DPOP, 1 },
    { OP_RPOP, 1 },
    { OP_INC, 1 },
    { OP_DEC, 1 }
};

int num_arg_counts = sizeof(arg_counts) / sizeof(arg_counts[0]);


typedef struct Options
{
    char *infile;
    char *outfile;
    char *symfile;
} Options;


typedef struct SymbolRef
{
    unsigned short location;
    struct SymbolRef *next;
} SymbolRef;


typedef struct Symbol
{
    char *name;
    unsigned short location;
    struct Symbol *next;
    SymbolRef *refs;
} Symbol;


typedef struct Context
{
    char *memory;
    unsigned short origin;
    Symbol *symbols;    // linked list of symbols
    int num_symbols;
    int line_number;
} Context;


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
        strcat(scratch, ".asm");
        options->infile = strdup(scratch);
    }
    else
    {
        options->infile = argv[1];
    }

    strcpy(scratch, options->infile);
    dot = strrchr(scratch, '.');
    *dot = 0;

    strcat(scratch, ".fo");
    options->outfile = strdup(scratch);

    *dot = 0;
    strcat(scratch, ".sym");
    options->symfile = strdup(scratch);

    return options;
}


void strip_comments(char *str)
{
    char *c = str;
    while (*c != ';' && *c != 0)
    {
        c++;
    }

    if (*c == ';')
    {
        *c = 0;
    }

    c--;

    while (c >= str)
    {
        if (isspace(*c))
        {
            *c = 0;
        }
        else
        {
            break;
        }
        c--;
    }
}


void print_error(Context *context, char *format, ...)
{
    va_list(args);
    printf("line %d: ", context->line_number);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}


char *skip_label(char *str)
{
    while (!isspace(*str) && *str != ':')
    {
        str += 1;
    }

    return str;
}


char *skip_white(char *str)
{
    while (isspace(*str))
    {
        str += 1;
    }

    return str;
}


void add_byte(Context *context, unsigned char val)
{
    context->memory[context->origin++] = val;
}


void add_word(Context *context, unsigned short val)
{
    add_byte(context, val >> 8);
    add_byte(context, val & 0xFF);
}


void add_literal(Context *context, char *literal)
{
    literal++;      // skip $
    unsigned short word = strtol(literal, NULL, 16);
    add_word(context, word);
}


Symbol *lookup_symbol(Context *context, char *name)
{
    Symbol *symbol = context->symbols;
    while (symbol != NULL)
    {
        if (!strcmp(symbol->name, name))
        {
            return symbol;
        }
        symbol = symbol->next;
    }
    return NULL;
}


Symbol *add_symbol(Context *context, char *name)
{
    Symbol *symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->location = 0xFFFF;
    symbol->next = context->symbols;
    symbol->refs = NULL;
    context->symbols = symbol;
    context->num_symbols++;
    return symbol;
}


void add_label_ref(Context *context, char *name)
{
    Symbol *symbol = lookup_symbol(context, name);
    if (symbol == NULL)
    {
        symbol = add_symbol(context, name);
    }

    SymbolRef *ref = malloc(sizeof(SymbolRef));
    ref->location = context->origin;
    ref->next = symbol->refs;
    symbol->refs = ref;

    add_byte(context, 0xFF);
    add_byte(context, 0xFF);
}


void add_string(Context *context, char *str)
{
    char *c;

    for (c = str; *c != 0; c++)
    {
        add_byte(context, *c);
    }
}


bool parse_pseudo(Context *context, int argc, char *argv[])
{
    if (!strcmp(argv[0], ".word"))
    {
        if (argv[1][0] == '$')
        {
            add_literal(context, argv[1]);
        }
        else
        {
            add_label_ref(context, argv[1]);
        }

        return TRUE;
    }

    if (!strcmp(argv[0], ".byte"))
    {
        add_byte(context, atoi(&(argv[1][1])));
        return TRUE;
    }

    if (!strcmp(argv[0], ".ascii"))
    {
        // Strip quotes
        char buf[MAXCHAR];
        strcpy(buf, &(argv[1][1]));
        char *pos = strrchr(buf, '"');
        *pos = 0;
        add_string(context, buf);
        return TRUE;
    }

    return FALSE;
}


bool verify_arg_count(Context *context, int argc, unsigned char opcode)
{
    int i;
    for (i = 0; i < num_arg_counts; i++)
    {
        if (arg_counts[i].opcode == opcode)
        {
            if (arg_counts[i].num_args == argc)
            {
                return TRUE;
            }
            else
            {
                print_error(context, "Incorrect number of arguments for %s, expected %d, saw %d.\n",
                        op_code_to_name(opcode), arg_counts[i].num_args, argc);
                return FALSE;
            }
        }
    }

    return TRUE;
}


bool add_register(Context *context, char *name)
{
    if (!strcmp(name, "IP"))
    {
        add_byte(context, REG_IP);
        return TRUE;
    }

    if (!strcmp(name, "CA"))
    {
        add_byte(context, REG_CA);
        return TRUE;
    }

    if (!strcmp(name, "X"))
    {
        add_byte(context, REG_X);
        return TRUE;
    }

    if (!strcmp(name, "Y"))
    {
        add_byte(context, REG_Y);
        return TRUE;
    }

    print_error(context, "Unknown register: |%s|\n", name);
    return FALSE;
}


bool is_register(char *str)
{
    if (!strcmp(str, "IP") || !strcmp(str, "CA") || !strcmp(str, "X") || !strcmp(str, "Y"))
    {
        return TRUE;
    }

    return FALSE;
}


bool is_literal(char *str)
{
    return (*str == '$');
}


// TODO - remove this?
bool is_symbol(Context *context, char *str)
{
    Symbol *symbol;
    for (symbol = context->symbols; symbol != NULL; symbol = symbol->next)
    {
        if (!strcmp(symbol->name, str))
        {
            return TRUE;
        }
    }

    return FALSE;
}


char *deref_argument(char *arg)
{
    static char inner[MAXCHAR];
    strcpy(inner, arg + 1);
    char *pos = strrchr(inner, ')');
    *pos = 0;
    return inner;
}


// TODO - switch parameter order
bool parse_opcode(char *opcode, Context *context)
{
    char buf[MAXCHAR];
    char *argv[MAXARGS];
    char *inner;
    int argc = 0;
    strcpy(buf, opcode);
    char *token = strtok(buf, SEPS);
    while(token)
    {
        argv[argc++] = token;
        token = strtok(NULL, SEPS);
    }

    if (argc == 0)
    {
        // How can this happen?
        printf("Malformed opcode: |%s|\n", opcode);
        return FALSE;
    }

    if (parse_pseudo(context, argc, argv))
    {
        return TRUE;
    }

    unsigned short code = op_name_to_code(argv[0]);
    if (code == 0)
    {
        print_error(context, "Unknown opcode: |%s|\n", argv[0]);
        return FALSE;
    }

    if (!verify_arg_count(context, argc - 1, code))
    {
        return FALSE;
    }

    // For LOAD, look at the addressing mode and adjust the code
    if ((code & 0xF0) == MASK_LOAD)
    {
        // TODO - break this out to a function that returns the mode, then switch on that
        if (is_register(argv[2]))
        {
            code = OP_LOAD0;
        }
        else if (is_literal(argv[2]) || argv[2][0] != '(')
        {
            code = OP_LOAD1;
        }
        else if (argv[2][0] == '(' && strrchr(argv[2], ')') != NULL)
        {
            inner = deref_argument(argv[2]);
            if (is_register(inner))
            {
                code = OP_LOAD2;
            }
            else
            {
                code = OP_LOAD3;
            }
        }
        else
        {
            print_error(context, "Unknown addressing mode: |%s|\n", argv[2]);
            return FALSE;
        }
    }

    add_byte(context, code);

    // Handle the first argument
    switch (code)
    {
        case OP_JMP:
            add_label_ref(context, argv[1]);
            break;

        case OP_LOAD0:
        case OP_LOAD1:
        case OP_LOAD2:
        case OP_LOAD3:
            if (!add_register(context, argv[1]))
            {
                return FALSE;
            }
            break;

        case OP_DPUSH:
        case OP_RPUSH:
        case OP_DPOP:
        case OP_RPOP:
        case OP_INC:
        case OP_DEC:
            if (!add_register(context, argv[1]))
            {
                return FALSE;
            }
            break;
    }

    // Handle the second argument (for codes that have a second argument)
    switch (code)
    {
        case OP_LOAD0:
            if (!add_register(context, argv[2]))
            {
                return FALSE;
            }
            break;

        case OP_LOAD1:
            if (argv[2][0] == '$')
            {
                add_literal(context, argv[2]);
            }
            else
            {
                add_label_ref(context, argv[2]);
            }
            break;

        case OP_LOAD2:
            inner = deref_argument(argv[2]);
            if (!add_register(context, inner))
            {
                return FALSE;
            }
            break;

        case OP_LOAD3:
            inner = deref_argument(argv[2]);
            if (inner[0] == '$')
            {
                add_literal(context, inner);
            }
            else
            {
                add_label_ref(context, inner);
            }
            break;
    }

    return TRUE;
}


bool parse_line(char *str, Context *context)
{
    strip_comments(str);
    if (strlen(str) == 0)
    {
        return TRUE;
    }

    char *label = NULL;
    char *opcode;
    if (!isspace(*str))
    {
        label = str;
        str = skip_label(str);
        *str = '\0';
        str += 1;
    }
    opcode = skip_white(str);

    if (label != NULL)
    {
        Symbol *symbol = lookup_symbol(context, label);
        if (symbol == NULL)
        {
            symbol = add_symbol(context, label);
        }
        symbol->location = context->origin;
    }

    if (*opcode == 0)
    {
        return TRUE;
    }

    return parse_opcode(opcode, context);
}


bool update_references(Context *context)
{
    printf("Updating references...\n");

    bool ok = TRUE;
    Symbol *symbol;
    SymbolRef *ref;
    for (symbol = context->symbols; symbol != NULL; symbol = symbol->next)
    {
        if (symbol->refs != NULL && symbol->location == 0xFFFF)
        {
            // TODO - extract the line number(s) from the refs
            printf("Undefined symbol: %s\n", symbol->name);
            ok = FALSE;
            continue;
        }

        char hi_byte = symbol->location >> 8;
        char lo_byte = symbol->location & 0xFF;
        for (ref = symbol->refs; ref != NULL; ref = ref->next)
        {
            context->memory[ref->location] = hi_byte;
            context->memory[ref->location + 1] = lo_byte;
        }
    }

    return ok;
}


bool assemble(FILE *in, Options *options)
{
    char str[MAXCHAR];
    Context *context = malloc(sizeof(Context));
    context->memory = malloc(MEMSIZE);
    context->origin = 0;
    context->symbols = NULL;
    context->num_symbols = 0;
    context->line_number = 0;

    puts("Assembling...");
    while (fgets(str, MAXCHAR, in) != NULL)
    {
        context->line_number += 1;
        if (!parse_line(str, context))
        {
            return FALSE;
        }
    }

    if (!update_references(context))
    {
        return FALSE;
    }

    // Write the binary image
    FILE *outfile = fopen(options->outfile, "wb");
    if (outfile == NULL)
    {
        printf("Could not open output file: %s\n", options->outfile);
        return FALSE;
    }

    fwrite(&(context->origin), sizeof(context->origin), 1, outfile);
    fwrite(context->memory, sizeof(char), context->origin, outfile);

    fclose(outfile);

    // Write the symbols (if we have any)
    if (context->symbols != NULL)
    {
        FILE *symfile = fopen(options->symfile, "w");
        if (symfile == NULL)
        {
            printf("Could not open symbol file: %s\n", options->symfile);
            return FALSE;
        }
        fprintf(symfile, "%d\n", context->num_symbols);
        Symbol *symbol;
        for (symbol = context->symbols; symbol != NULL; symbol = symbol->next)
        {
            fprintf(symfile, "%04X %s\n", symbol->location, symbol->name);
        }
        fclose(symfile);
    }

    return TRUE;
}


int main(int argc, char *argv[])
{
    Options *options = parse_args(argc, argv);
    if (options == NULL)
    {
        return 1;
    }

    FILE *infile = fopen(options->infile, "r");
    if (infile == NULL)
    {
        printf("Could not open input file: %s\n", options->infile);
        return 1;
    }

    int ok = assemble(infile, options);

    fclose(infile);

    if (!ok)
    {
        printf("Assembly FAILED.\n");
    }

    return 0;
}

