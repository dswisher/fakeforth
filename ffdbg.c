
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "simulator.h"

#define MAXARGS 20
#define SEPS " \t\n"

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

    strcpy(scratch, options->infile);
    dot = strrchr(scratch, '.');
    *dot = 0;

    strcat(scratch, ".fsym");
    options->symfile = strdup(scratch);

    return options;
}


bool parse_and_execute_command(Simulator *sim, int argc, char *argv[])
{
    // TODO - turn this into a command table or some such
    if (!strcmp(argv[0], "q") || !strcmp(argv[0], "quit"))
    {
        return FALSE;
    }

    if (!strcmp(argv[0], "n") || !strcmp(argv[0], "next"))
    {
        sim_step(sim);
        return TRUE;
    }

    if (!strcmp(argv[0], "r") || !strcmp(argv[0], "run"))
    {
        sim_run(sim);
        return TRUE;
    }

    if (!strcmp(argv[0], "p") || !strcmp(argv[0], "print"))
    {
        // TODO - accept additional args to indicate what to print
        sim_print(sim);
        return TRUE;
    }

    // TODO - add help command

    if (!strcmp(argv[0], "list"))
    {
        sim_disassemble(sim, sim->pc, 1);
        return TRUE;
    }

    return TRUE;
}


bool execute_command(Simulator *sim)
{
    fputs("(ffdbg) ", stdout);
    char buf[MAXCHAR];
    char *args[MAXARGS];
    if (fgets(buf, MAXCHAR, stdin) == NULL)
    {
        // ignore blank lines
        return TRUE;
    }

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

    return parse_and_execute_command(sim, num, args);
}


int main(int argc, char *argv[])
{
    Options *options = parse_args(argc, argv);
    if (options == NULL)
    {
        return 1;
    }

    Simulator *sim = sim_init(options->infile);
    sim_load_symbols(sim, options->symfile);

    while (execute_command(sim))
    {
    }

    // TODO - prompt for commands and run 'em
}

