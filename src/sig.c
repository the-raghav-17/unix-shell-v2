#include "sig.h"
#include "job.h"

#include <signal.h>
#include <stdlib.h>


void
block_sigchld(void)
{
    sigset_t block_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block_set, NULL);
}


void
unblock_sigchld(void)
{
    sigset_t block_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &block_set, NULL);
}


void
set_sigchld_disposition(void)
{
    struct sigaction action;

    action.sa_handler = handle_async_jobs;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGCHLD, &action, NULL);
}


void
ignore_sigchld(void)
{
    struct sigaction action;

    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGCHLD, &action, NULL);
}


void
reset_signal_disposition(void)
{
    struct sigaction action;

    action.sa_handler = SIG_DFL;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);
    sigaction(SIGTTIN, &action, NULL);
    sigaction(SIGTTOU, &action, NULL);
    ignore_sigchld();
}
