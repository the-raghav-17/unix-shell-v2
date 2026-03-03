#include "builtin_helper.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


int
builtin_exec(char **argv, int argc)
{
    if (argc == 2) {
        fprintf(stderr, "shell: exec: Expected arguments\n");
        return 1;
    }

    if (argc > 2) {
        if (execvp(argv[1], argv + 1) == -1) {
            perror("shell: exec:");
            return 1;
        }

        exit(EXIT_SUCCESS);
    }

    return 0;
}
