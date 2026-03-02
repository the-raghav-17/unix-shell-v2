#include "builtin_helper.h"
#include "job.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


int
builtin_exit(char **argv, int argc)
{
    /* If user wants to exit and there are still
       jobs running in background, we will warn
       user about the jobs just once. If user still
       wants to exit, we will terminate the jobs,
       collect return status and then exit the shell. */

    if (argc > 2) {
        fprintf(stderr, "shell: exit: Too many arguments\n");
        return 1;
    }

    if (get_job_head() != NULL) {
        printf("Terminating all jobs...\n");
        terminate_all_jobs();
    }
    printf("[Exiting...]\n");
    exit(EXIT_SUCCESS);
    return 0;
}
