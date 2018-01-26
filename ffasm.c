
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"
#include "simulator.h"
#include "opcodes.h"
#include "util.h"
#include "forth.h"


#define OK 0
#define ERROR 1
#define UNHANDLED 2


typedef struct ArgCount
{
    unsigned char opcode;
    int num_args;
} ArgCount;

ArgCount arg_counts[] =
{
    { OP_JMP, 1 },
    { OP_JEQ, 1 },
    { OP_JNE, 1 },
    { OP_JGT, 1 },
    { OP_JLT, 1 },
    { OP_JGE, 1 },
    { OP_JLE, 1 },
    { OP_LDW, 2 },
    { OP_LDB, 2 },
    { OP_STW, 2 },
    { OP_STB, 2 },
    { OP_DPUSH, 1 },
    { OP_RPUSH, 1 },
    { OP_DPOP, 1 },
    { OP_RPOP, 1 },
    { OP_INC, 1 },
    { OP_DEC, 1 },
    { OP_NEG, 1 },
    { OP_GETC, 1 },
    { OP_PUTC, 1 },
    { OP_PUTN, 1 },
    { OP_PUTS, 1 },
    { OP_PSTACK, 0 },
    { OP_DCLR, 0 },
    { OP_RCLR, 0 },
    { OP_BRK, 0 },
    { OP_ADD, 2 },
    { OP_AND, 2 },
    { OP_OR, 2 },
    { OP_XOR, 2 },
    { OP_MUL, 2 },
    { OP_SUB, 2 },
    { OP_CALL, 1 },
    { OP_CMP, 2 },
    { OP_RET, 0 }
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
    int line_number;
    struct SymbolRef *next;
} SymbolRef;


typedef struct Variable
{
    char *name;
    unsigned short value;
    struct Variable *next;
} Variable;


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
    Variable *variables;        // linked list of variables
    Symbol *symbols;            // linked list of symbols
    int num_symbols;
    int line_number;
    unsigned short last_dict;   // address of last dict entry
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
        options->infile = my_strdup(scratch);
    }
    else
    {
        options->infile = argv[1];
    }

    strcpy(scratch, options->infile);
    dot = strrchr(scratch, '.');
    *dot = 0;

    strcat(scratch, ".fo");
    options->outfile = my_strdup(scratch);

    *dot = 0;
    strcat(scratch, ".sym");
    options->symfile = my_strdup(scratch);

    return options;
}


void strip_comments(char *str)
{
    char *c = str;
    while (*c != ';' && *c != 0)
    {
        if (*c == '"')
        {
            c++;
            while (*c != '"' && *c != 0)
            {
                c++;
            }
            if (*c == '"')
            {
                c++;
            }
        }
        else
        {
            c++;
        }
    }

    if (*c == ';')
    {
        *c = 0;
    }

    c--;

    // Remove trailing whitespace
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


void add_space(Context *context, unsigned short num)
{
    context->origin += num;
}


void add_word(Context *context, unsigned short val)
{
    add_byte(context, val >> 8);
    add_byte(context, val & 0xFF);
}


void add_literal(Context *context, char *literal)
{
    literal++;      // skip $
    if (*literal == '\'')
    {
        // TODO - handle escape sequences (like "\n")
        literal++;
        add_word(context, *literal);
    }
    else
    {
        unsigned short word = strtol(literal, NULL, 16);
        add_word(context, word);
    }
}


Variable *set_variable(Context *context, char *name, char *val)
{
    unsigned short word = strtol(val + 1, NULL, 16);    // val + 1 to skip leading $

    // If the var already exists, just update the value
    Variable *var;
    for (var = context->variables; var != NULL; var = var->next)
    {
        if (!strcmp(var->name, name))
        {
            var->value = word;
            return var;
        }
    }

    // Var does not exist; create it.
    var = malloc(sizeof(Variable));
    var->next = context->variables;
    var->name = my_strdup(name);
    var->value = word;
    context->variables = var;

    return var;
}


Variable *lookup_variable(Context *context, char *name)
{
    Variable *var = context->variables;
    while (var != NULL)
    {
        if (!strcmp(var->name, name))
        {
            return var;
        }
        var = var->next;
    }
    return NULL;
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
    symbol->name = my_strdup(name);
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
    ref->line_number = context->line_number;
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


bool check_arg_count(Context *context, char *name, int seen, int needed)
{
    if (seen < needed)
    {
        print_error(context, "Incorrect number of arguments to '%s': need at least %d, but only got %d.\n", name, needed, seen);
        return FALSE;
    }

    return TRUE;
}


int parse_pseudo(Context *context, int argc, char *argv[])
{
    if (!strcmp(argv[0], ".word"))
    {
        if (!check_arg_count(context, argv[0], argc, 1))
        {
            return ERROR;
        }

        if (argv[1][0] == '$')
        {
            add_literal(context, argv[1]);
        }
        else
        {
            add_label_ref(context, argv[1]);
        }

        return OK;
    }

    if (!strcmp(argv[0], ".byte"))
    {
        if (!check_arg_count(context, argv[0], argc, 1))
        {
            return ERROR;
        }

        unsigned char val = strtol(&(argv[1][1]), NULL, 16);
        add_byte(context, val);
        return OK;
    }

    if (!strcmp(argv[0], ".ascii"))
    {
        if (!check_arg_count(context, argv[0], argc, 1))
        {
            return ERROR;
        }

        add_string(context, argv[1]);
        return OK;
    }

    if (!strcmp(argv[0], ".asciz"))
    {
        if (!check_arg_count(context, argv[0], argc, 1))
        {
            return ERROR;
        }

        add_string(context, argv[1]);
        add_byte(context, 0);
        return OK;
    }

    if (!strcmp(argv[0], ".set"))
    {
        if (!check_arg_count(context, argv[0], argc, 2))
        {
            return ERROR;
        }

        set_variable(context, argv[1], argv[2]);
        return OK;
    }

    if (!strcmp(argv[0], ".space"))
    {
        if (!check_arg_count(context, argv[0], argc, 1))
        {
            return ERROR;
        }

        unsigned short num = strtol(&(argv[1][1]), NULL, 16);
        add_space(context, num);
        return OK;
    }

    if (!strcmp(argv[0], ".dict"))
    {
        if (!check_arg_count(context, argv[0], argc, 2))
        {
            return ERROR;
        }

        char *name = argv[1];
        unsigned char len = strlen(name);

        if (argc > 2)
        {
            for (int i = 2; i < argc; i++)
            {
                if (!strcmp(argv[i], "IMMED"))
                {
                    len |= F_IMMED;
                }
                else if (!strcmp(argv[i], "HIDDEN"))
                {
                    len |= F_HIDDEN;
                }
                else
                {
                    print_error(context, "Unexpected dict flag: %s\n", argv[i]);
                    return ERROR;
                }
            }
        }

        // Save the current addr
        unsigned short addr = context->origin;

        // Set up the entry
        add_word(context, context->last_dict);  // pointer to prev word
        add_byte(context, len);                 // length + flags
        add_string(context, name);

        // Remember where to link the next word
        context->last_dict = addr;

        return OK;
    }

    if (!strcmp(argv[0], ".lastdict"))
    {
        // Equivalent to ".word last_dict"
        add_word(context, context->last_dict);
        return OK;
    }

    return UNHANDLED;
}


bool verify_arg_count(Context *context, int argc, unsigned char opcode)
{
    for (int i = 0; i < num_arg_counts; i++)
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
    unsigned char reg = op_name_to_register(name);

    if (reg > 0)
    {
        add_byte(context, reg);
        return TRUE;
    }

    print_error(context, "Unknown register: |%s|\n", name);
    return FALSE;
}


bool is_literal(char *str)
{
    return (*str == '$');
}


char *deref_argument(char *arg)
{
    static char inner[MAXCHAR];
    strcpy(inner, arg + 1);
    char *pos = strrchr(inner, ')');
    *pos = 0;
    return inner;
}


int parse_address_mode(Context *context, char *arg)
{
    if (op_is_register(arg))
    {
        return ADDR_MODE0;
    }

    if (is_literal(arg) || arg[0] != '(')
    {
        return ADDR_MODE1;
    }

    if (arg[0] == '(' && strrchr(arg, ')') != NULL)
    {
        char *inner = deref_argument(arg);
        if (op_is_register(inner))
        {
            return ADDR_MODE2;
        }
        else
        {
            return ADDR_MODE3;
        }
    }

    print_error(context, "Unknown addressing mode: |%s|\n", arg);
    return 0xFF;
}


bool add_by_mode(Context *context, int mode, char *arg)
{
    char *inner;
    Variable *var;
    switch (mode)
    {
        case ADDR_MODE0:
            return add_register(context, arg);

        case ADDR_MODE1:
            if (arg[0] == '$')
            {
                add_literal(context, arg);
                return TRUE;
            }
            else if ((var = lookup_variable(context, arg)) != NULL)
            {
                add_word(context, var->value);
                return TRUE;
            }
            else
            {
                add_label_ref(context, arg);
                return TRUE;
            }

        case ADDR_MODE2:
            inner = deref_argument(arg);
            return add_register(context, inner);

        case ADDR_MODE3:
            inner = deref_argument(arg);
            if (inner[0] == '$')
            {
                add_literal(context, inner);
                return TRUE;
            }
            else
            {
                add_label_ref(context, inner);
                return TRUE;
            }
    }

    print_error(context, "Unhandled address mode: %d\n", mode);
    return FALSE;
}


void split_params(char *str, char *argv[], int *argc)
{
    static char buf[MAXCHAR];

    strcpy(buf, str);
    *argc = 0;
    char *c = buf;
    while (TRUE)
    {
        // Skip whitespace
        while (*c == ' ' || *c == '\t' || *c == ',')
        {
            c++;
        }

        // Stop if we're out of string
        if (*c == 0)
        {
            return;
        }

        // Parse an argument, possibly quoted
        if (*c == '"')
        {
            c++;
            argv[(*argc)++] = c;
            while (*c != '"' && *c != 0)
            {
                c++;
            }
            if (*c != 0)
            {
                *c = 0;
                c++;
            }
        }
        else
        {
            argv[(*argc)++] = c;
            while (*c != ' ' && *c != '\t' && *c != ',' && *c != 0)
            {
                c++;
            }
            if (*c != 0)
            {
                *c = 0;     // terminate
                c++;
            }
        }
    }
}


bool parse_opcode(Context *context, char *opcode)
{
    char *argv[MAXARGS];
    int argc = 0;
    int status;

    split_params(opcode, argv, &argc);

    if (argc == 0)
    {
        // How can this happen?
        printf("Malformed opcode: |%s|\n", opcode);
        return FALSE;
    }

    status = parse_pseudo(context, argc, argv);
    if (status == OK)
    {
        return TRUE;
    }
    else if (status == ERROR)
    {
        return FALSE;
    }
    else if (status != UNHANDLED)
    {
        printf("Unexpected return value from parse_pseudo: %d\n", status);
        return FALSE;
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

    // Determine the addressing mode
    int mode = ADDR_MODE0;  // default for most opcodes
    switch (code)
    {
        case OP_JMP:
        case OP_JEQ:
        case OP_JNE:
        case OP_JGT:
        case OP_JLT:
        case OP_JGE:
        case OP_JLE:
            mode = parse_address_mode(context, argv[1]);
            break;

        case OP_LDW:
        case OP_LDB:
        case OP_ADD:
        case OP_AND:
        case OP_OR:
        case OP_XOR:
        case OP_MUL:
        case OP_SUB:
        case OP_CMP:
            mode = parse_address_mode(context, argv[2]);
            break;

        case OP_STW:
        case OP_STB:
            mode = parse_address_mode(context, argv[2]);
            if (mode == ADDR_MODE1)
            {
                print_error(context, "Unsupported address mode %d for STORE.\n", mode);
                return FALSE;
            }
            break;
    }

    if (mode == 0xFF)
    {
        return FALSE;
    }

    // Write the op-code and addressing mode
    add_byte(context, code | mode);

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
            if (!add_by_mode(context, mode, argv[1]))
            {
                return FALSE;
            }
            break;

        case OP_CALL:
            // TODO - should we allow other modes? (If so, revisit sim_step_over!)
            add_label_ref(context, argv[1]);
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
        case OP_GETC:
        case OP_PUTC:
        case OP_PUTN:
        case OP_PUTS:
        case OP_ADD:
        case OP_AND:
        case OP_OR:
        case OP_XOR:
        case OP_MUL:
        case OP_SUB:
        case OP_CMP:
            if (!add_register(context, argv[1]))
            {
                return FALSE;
            }
            break;
    }

    // Handle the second argument (for codes that have a second argument)
    switch (code)
    {
        case OP_LDW:
        case OP_LDB:
        case OP_STW:
        case OP_STB:
        case OP_ADD:
        case OP_AND:
        case OP_OR:
        case OP_XOR:
        case OP_MUL:
        case OP_SUB:
        case OP_CMP:
            if (!add_by_mode(context, mode, argv[2]))
            {
                return FALSE;
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

    return parse_opcode(context, opcode);
}


bool update_references(Context *context)
{
    printf("Updating references...\n");

    bool ok = TRUE;
    Symbol *symbol;
    SymbolRef *ref;
    char lines[MAXCHAR];
    char scratch[20];
    for (symbol = context->symbols; symbol != NULL; symbol = symbol->next)
    {
        if (symbol->refs != NULL && symbol->location == 0xFFFF)
        {
            strcpy(lines, "");
            for (ref = symbol->refs; ref != NULL; ref = ref->next)
            {
                if (strlen(lines) > 0)
                {
                    strcat(lines, ", ");
                }
                sprintf(scratch, "%d", ref->line_number);
                strcat(lines, scratch);
            }
            printf("Undefined symbol: %s, line %s\n", symbol->name, lines);
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
    context->variables = NULL;
    context->num_symbols = 0;
    context->line_number = 0;
    context->last_dict = 0;

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

