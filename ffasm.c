
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


#define MAXCHAR 200     // Max length of one line
#define MEMSIZE 1<<16   // TODO - move out to common header


typedef struct Options
{
    char *infile;
    char *outfile;
} Options;


Options *parse_args(int argc, char *argv[])
{
    printf("MEMSIZE: %d\n", MEMSIZE);

    if (argc < 3)
    {
        printf("Too few arguments!\n");
        printf("Usage: %s <infile> <outfile>\n", argv[0]);
        return NULL;
    }

    Options *options = malloc(sizeof(Options));

    options->infile = argv[1];
    options->outfile = argv[2];

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


void parse_line(char *str, char *memory, int *origin)
{
    strip_comments(str);
    if (strlen(str) == 0)
    {
        return;
    }

    printf("|%s|\n", str);
    *(memory + *origin) = *origin;
    *origin += 2;
}


int assemble(FILE *in, FILE *out)
{
    char str[MAXCHAR];
    char *memory = malloc(MEMSIZE);
    int origin = 0;     // next address to be written

    while (fgets(str, MAXCHAR, in) != NULL)
    {
        parse_line(str, memory, &origin);
    }

    fwrite(memory, sizeof(char), origin, out);
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

    assemble(infile, outfile);

    fclose(infile);
    fclose(outfile);
}

