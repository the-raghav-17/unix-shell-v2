#include "user.h"

#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>


struct passwd *
get_pw_struct(void)
{
    static struct passwd *pw = NULL;
    if (pw == NULL) {
        errno = 0;
        pw = getpwuid(getuid());
        if (pw == NULL) {
            perror("getpwuid");
            return NULL;
        }
    }

    return pw;
}


char *
get_user_home_dir(void)
{
    struct passwd *pw = get_pw_struct();
    if (pw == NULL) {
        return NULL;
    }
    return pw->pw_dir;
}
