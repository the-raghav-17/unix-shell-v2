#include "builtin_helper.h"
#include "user.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>


int
builtin_cd(char **argv, int argc)
{
    assert(argc > 1 && argv != NULL);
    /* argc includes all command args and NULL */

    /* User typed cd w/o any args */
    if (argc == 2) {
        char *user_home_dir = get_user_home_dir();
        if (user_home_dir == NULL) {
            fprintf(stderr, "shell: cd: Failed to change directory");
            return 1;
        }
        if (chdir(user_home_dir) == -1) {
            perror("shell: cd");
            return 1;
        }
        return 0;
    }

    /* User typed cd w/ args */
    if (argc == 3) {
        if (chdir(argv[1]) == -1) {
            perror("shell: cd");
            return 1;
        }
        return 0;
    }

    if (argc > 3) {
        fprintf(stderr, "shell: cd: Too many arguments\n");
        return 1;
    }

    return 0;
}
