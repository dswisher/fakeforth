
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#define TRUE 1
#define FALSE 0
#define MAXCHAR 200     // Max length of one line
#define MEMSIZE 1<<16   // TODO - move out to common header


typedef struct Options
{
    char *infile;
    char *outfile;
    char *mapfile;
} Options;


typedef struct SymbolRef
{
    int location;
    struct SymbolRef *next;
} SymbolRef;


typedef struct Symbol
{
    char *name;
    int location;
    struct Symbol *next;
    SymbolRef *refs;
} Symbol;


typedef struct Context
{
    char *memory;
    int origin;
    Symbol *symbols;    // linked list of symbols
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
        strcat(scratch, ".fa");
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
    strcat(scratch, ".fd");
    options->mapfile = strdup(scratch);

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


char *skip_label(char *str)
{
    char *word = str;

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


void add_byte(Context *context, char val)
{
    context->memory[context->origin++] = val;
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
    symbol->location = 0;
    symbol->next = context->symbols;
    symbol->refs = NULL;
    context->symbols = symbol;
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
}


int parse_opcode(char *opcode, Context *context)
{
    char *arg;
    if ((arg = strchr(opcode, ' ')) != NULL)
    {
        *arg = 0;
        arg += 1;
    }

    if (!strcmp(opcode, "NOP"))
    {
        add_byte(context, 0x01);
    }
    else if (!strcmp(opcode, "HLT"))
    {
        add_byte(context, 0xFF);
    }
    else if (!strcmp(opcode, "JMP"))
    {
        add_byte(context, 0x02);
        add_label_ref(context, arg);
        add_byte(context, 0xEE);
        add_byte(context, 0xEE);
    }
    else
    {
        printf("Unknown opcode: |%s|\n", opcode);
        return FALSE;
    }

    return TRUE;
}


int parse_line(char *str, Context *context)
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

    return parse_opcode(opcode, context);
}


void update_references(Context *context)
{
    printf("Updating references...\n");

    Symbol *symbol;
    SymbolRef *ref;
    for (symbol = context->symbols; symbol != NULL; symbol = symbol->next)
    {
        char hi_byte = symbol->location >> 8;
        char lo_byte = symbol->location & 0xFF;
        // printf("Symbol: |%s|, location: 0x%04X, hi: %02X, lo: %02X\n", symbol->name, symbol->location, hi_byte, lo_byte);
        for (ref = symbol->refs; ref != NULL; ref = ref->next)
        {
            context->memory[ref->location] = hi_byte;
            context->memory[ref->location + 1] = lo_byte;
        }
    }
}


int assemble(FILE *in, FILE *out)
{
    char str[MAXCHAR];
    Context *context = malloc(sizeof(Context));
    context->memory = malloc(MEMSIZE);
    context->origin = 0;
    context->symbols = NULL;

    puts("Assembling...");
    while (fgets(str, MAXCHAR, in) != NULL)
    {
        if (!parse_line(str, context))
        {
            return FALSE;
        }
    }

    update_references(context);

    fwrite(context->memory, sizeof(char), context->origin, out);

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

    FILE *outfile = fopen(options->outfile, "wb");
    if (outfile == NULL)
    {
        printf("Could not open output file: %s\n", options->outfile);
        return 1;
    }

    int ok = assemble(infile, outfile);

    fclose(infile);
    fclose(outfile);

    if (!ok)
    {
        printf("Assembly FAILED.\n");
    }
}

