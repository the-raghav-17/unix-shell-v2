#include "builtin_helper.h"
#include "builtin.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>


Builtin
match_builtin(char **argv)
{
    assert(argv != NULL && argv[0] != NULL);

    if (!strcmp(argv[0], "fg"))   return BUILTIN_FG;
    if (!strcmp(argv[0], "bg"))   return BUILTIN_BG;
    if (!strcmp(argv[0], "jobs")) return BUILTIN_JOBS;
    if (!strcmp(argv[0], "cd"))   return BUILTIN_CD;
    if (!strcmp(argv[0], "exit")) return BUILTIN_EXIT;
    if (!strcmp(argv[0], "exec")) return BUILTIN_EXEC;

    return BUILTIN_NONE;
}


int
exec_builtin(Builtin builtin, char **argv, int argc, int infile, int outfile)
{
    assert(builtin != BUILTIN_NONE);
    assert(argv != NULL && argv[0] != NULL);

    int stdin_temp = -1;
    int stdout_temp = -1;
    if (infile != STDIN_FILENO) {
        stdin_temp = dup(STDIN_FILENO);
        dup2(infile, STDIN_FILENO);
    }
    if (outfile != STDOUT_FILENO) {
        stdout_temp = dup(STDOUT_FILENO);
        dup2(outfile, STDOUT_FILENO);
    }

    int return_val;

    switch (builtin) {
        case BUILTIN_FG:
            return_val = builtin_fg(argv, argc);
            break;

        case BUILTIN_BG:
            return_val = builtin_bg(argv, argc);
            break;

        case BUILTIN_JOBS:
            return_val = builtin_jobs(argv, argc);
            break;

        case BUILTIN_CD:
            return_val = builtin_cd(argv, argc);
            break;

        case BUILTIN_EXIT:
            return_val = builtin_exit(argv, argc);
            break;

        case BUILTIN_EXEC:
            return_val = builtin_exec(argv, argc);
            break;

        default:
            return_val = 1;
            break;
    }

    if (stdin_temp != -1) {
        dup2(stdin_temp, STDIN_FILENO);
    }
    if (stdout_temp != -1) {
        dup2(stdout_temp, STDOUT_FILENO);
    }

    return return_val;
}
