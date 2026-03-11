#include "process.h"
#include "sig.h"

#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


Process *
get_process_obj(void)
{
    Process *process = malloc(sizeof(*process));
    if (process == NULL) {
        perror("get_Process_obj");
        return NULL;
    }

    process->argv       = NULL;
    process->argc       = 0;
    process->capacity   = 0;
    process->return_val = 0;
    return process;
}


void
destroy_process_obj(Process *process)
{
    for (int i = 0; i < process->argc; i++) {
        free(process->argv[i]);
    }
    free(process->argv);
    free(process);
}


void
add_arg_to_process_string(Process *process, char *arg)
{
    if (arg == NULL) {
        return;
    }

    if (process->argc == 1) {
        strncpy(process->string, arg, sizeof(process->string));
    }
    else {
        /* Add a space */
        strncat(process->string, " ", sizeof(process->string));

        /* Add the new argument */
        strncat(process->string, arg, sizeof(process->string));
    }
}


int
add_arg_to_process(Process *process, const char *arg)
{
    #define INCR_SIZE 2

    if (process->capacity <= process->argc) {
        /* Increase capacity for insufficient size */
        process->capacity += INCR_SIZE;
        
        char **temp = realloc(process->argv, sizeof(*temp) * (process->capacity));
        if (temp == NULL) {
            process->capacity -= 1; /* reset capacity */
            perror("add_arg_to_Process");
            return -1;
            /* Process->argv still points to prev memory */
        }
    
        process->argv = temp;
    }

    process->argv[process->argc] = arg;
    process->argc += 1;

    add_arg_to_process_string(process, arg);

    /* Return index where arg is stored */
    return process->argc - 1;

    #undef INCR_SIZE
}


#define UPDATE_FDS(infile, outfile)        \
    do {                                   \
        if (infile != STDIN_FILENO) {      \
            dup2(infile, STDIN_FILENO);    \
            close(infile);                 \
        }                                  \
        if (outfile != STDOUT_FILENO) {    \
            dup2(outfile, STDOUT_FILENO);  \
            close(outfile);                \
        }                                  \
    } while (false);

void
launch_process(Process *process, int infile, int outfile)
{
    UPDATE_FDS(infile, outfile);
    reset_signal_disposition();

    char **argv = process->argv;
    execvp(argv[0], argv);

    /* execvp fails */
    char buf[32];
    snprintf(buf, sizeof(buf), "%s", argv[0]);
    if (errno == ENOENT) {
        fprintf(stderr, "%s: Command not found\n", buf);
    }
    else {
        perror(buf);
    }
    _exit(EXIT_FAILURE);
}

#undef UPDATE_FDS
