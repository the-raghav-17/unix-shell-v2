#include "shell.h"
#include "executor.h"
#include "input.h"
#include "lexer.h"
#include "list.h"
#include "parser.h"
#include "token.h"
#include "user.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>


static void set_signals_to_ignore(void);
static int put_shell_in_new_group(void);
static int init_shell(void);
static void display_prompt(void);
static void start_shell_loop(void);


typedef struct Shell_param
{
    pid_t shell_pgid;
    int   shell_terminal;
    struct termios shell_tmodes;
} Shell_param;

static Shell_param shell_param;


int
get_shell_terminal(void)
{
    return shell_param.shell_terminal;
}


static void
set_signals_to_ignore(void)
{
    struct sigaction action;

    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    // sigaction(SIGINT, &action, NULL);  for now turning it off
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);
    sigaction(SIGTTIN, &action, NULL);
    sigaction(SIGTTOU, &action, NULL);
}


static int
put_shell_in_new_group(void)
{
    shell_param.shell_pgid = getpid();
    if (setpgid(shell_param.shell_pgid, shell_param.shell_pgid) == -1) {
        perror("Couldn't put the shell in new process group");
        return -1;
    }

    return 0;
}


void
put_shell_in_foreground(void)
{
    /* Put shell in foreground */
    tcsetpgrp(shell_param.shell_terminal, shell_param.shell_pgid);

    /* Restore shell's terminal settings */
    tcsetattr(get_shell_terminal(), TCSANOW, &(shell_param.shell_tmodes));
}


#define IS_SHELL_IN_FOREGROUND() \
        (tcgetpgrp(shell_param.shell_terminal) == getpgrp())  /* compare foreground group with shell's group */

static int
init_shell(void)
{
    shell_param.shell_terminal = open("/dev/tty", O_RDONLY);

    /* Stop the process group the shell belongs to if started in background */
    while (!IS_SHELL_IN_FOREGROUND()) {
        kill(0, SIGTTIN);
    }

    if (put_shell_in_new_group() == -1) {
        return -1;
    }
    set_signals_to_ignore();

    /* Get terminal settings */
    tcgetattr(get_shell_terminal(), &(shell_param.shell_tmodes));

    /* Grab control of the terminal */
    put_shell_in_foreground();
    return 0;
}

#undef IS_SHELL_IN_FOREGROUND


#define TEXT_COLOR_START "\e[32m"  /* green color */
#define TEXT_COLOR_END   "\e[0m"

static void
display_prompt(void)
{
    char *username = get_user_name();
    if (username == NULL) {
        username = "user";  /* fallback username */
    }

    char hostname[32];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname");
        snprintf(hostname, sizeof(hostname), "host"); /* fallback hostname */
    }

    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        perror("getcwd");
        cwd = "<null>";
    }

    printf(TEXT_COLOR_START"%s@%s: %s\n"TEXT_COLOR_END, username, hostname, cwd);
    printf("$> ");
    if (strcmp(cwd, "<null>")) {
        free(cwd);
    }
}

#undef TEXT_COLOR_START
#undef TEXT_COLOR_END


static void
start_shell_loop(void)
{
    while (1) {
        display_prompt();
        char *line = read_from_stdin();
        if (line[0] == '\0' || line == NULL) {
            /* If user clicks enter without typing anything or on error */
            free(line);
            continue;
        }

        Token *tokens = tokenize(line);
        if (tokens == NULL) {
            free(line);
            continue;
        }

        List_node *list_head = parse_tokens(tokens);
        if (list_head == NULL) {
            free(tokens);
            free(line);
            continue;
        }

        execute(list_head);

        destroy_list(list_head);
        free(tokens);
        free(line);
    }
}


int
start_shell(void)
{
    if (init_shell() == -1) {
        return EXIT_FAILURE;
    }

    start_shell_loop();
    return EXIT_SUCCESS;
}
