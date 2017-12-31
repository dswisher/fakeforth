
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "simulator.h"


typedef struct Options
{
    char *infile;
    char *symfile;
} Options;


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

    return options;
}



int main(int argc, char *argv[])
{
    Options *options = parse_args(argc, argv);
    if (options == NULL)
    {
        return 1;
    }

    Simulator *sim = sim_init(options->infile);

    sim_run(sim);

    return 0;
}

