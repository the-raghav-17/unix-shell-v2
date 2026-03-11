#include "pipeline.h"
#include "process.h"
#include "shell.h"
#include "sig.h"
#include "job.h"
#include "builtin.h"

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


static int handle_pipeline_suspension(Pipeline *pipeline);
static void handle_pipeline_termination(Pipeline *pipeline, int sig, pid_t pid);
static Pipe_return_status wait_for_pipeline(Pipeline *pipeline, bool in_subshell);
static int create_and_exec_child_process(Pipeline *pipeline, int index, int infile, int outfile, bool is_last_proc, int pipefd[], bool in_subshell);
static int setup_and_launch_pipeline(Pipeline *pipeline, bool in_subshell);



/* ========== Helper functions for the pipeline object ========== */


Pipeline *
get_pipeline_obj(void)
{
    Pipeline *pipeline = malloc(sizeof(*pipeline));
    if (pipeline == NULL) {
        perror("get_pipeline_obj");
        return NULL;
    }

    pipeline->process       = NULL;
    pipeline->gid           = 0;
    pipeline->process_count = 0;
    pipeline->capacity      = 0;
    pipeline->is_running    = false;
    return pipeline;
}


void
destroy_pipeline_obj(Pipeline *pipeline)
{
    /* Destroy each process in the pipeline, only
    if pipeline is not managed by job handler */
    if (!pipeline->is_running) {
        for (int i = 0; i < pipeline->process_count; i++) {
            destroy_process_obj(pipeline->process[i]);
        }
        free(pipeline->process);
    }

    free(pipeline);
}


#define SAFE_CONCAT_SIZE (sizeof(pipeline->string) - strlen(pipeline->string) - 1)

void
add_process_to_pipeline_string(Pipeline *pipeline, Process *process)
{
    if (pipeline->process_count == 1) {
        strncpy(pipeline->string, process->string, sizeof(pipeline->string));
    }
    else {
        /* Insert the pipe character */
        strncat(pipeline->string, " | ", SAFE_CONCAT_SIZE);

        /* Insert the process string */
        strncat(pipeline->string, process->string, SAFE_CONCAT_SIZE);
    }
}

#undef SAFE_CONCAT_SIZE


int
add_process_to_pipeline(Pipeline *pipeline, Process *process)
{
    #define INCR_SIZE 1

    if (pipeline->capacity <= pipeline->process_count) {
        pipeline->capacity += INCR_SIZE;

        Process **temp = realloc(pipeline->process, sizeof(*temp) * (pipeline->capacity));
        if (temp == NULL) {
            pipeline->capacity -= 1; /* reset capacity */
            perror("add_Process_to_pipeline");
            return -1;
        }

        pipeline->process = temp;
    }

    /* Store the Process obj in array */
    pipeline->process[pipeline->process_count] = process;
    pipeline->process_count += 1;

    add_process_to_pipeline_string(pipeline, process);

    /* Return index where the Process was stored */
    return pipeline->process_count - 1;

    #undef INCR_SIZE
}


/* ========== Pipeline execution and handling functions ========== */


static void
handle_pipeline_termination(Pipeline *pipeline, int sig, pid_t pid)
{
    /* Make sure all processes in pipeline are terminated */
    for (int i = 0; i < pipeline->process_count; i++) {
        pid_t process_id = pipeline->process[i]->pid;
        if (process_id == pid) {
            continue;
        }

        kill(process_id, sig);
    }
    pipeline->is_running = false;
}


/* Suspends a pipeline. On success, returns 0. On failure
    returns -1, and continues every process in pipeline. */
static int
handle_pipeline_suspension(Pipeline *pipeline)
{
    /* Make sure every process in the pipeline
       is suspended, even if only one was supposed to. */
    kill(-pipeline->gid, SIGTSTP);

    bool is_stopped    = true;
    bool in_foreground = false;
    Job *job = add_pipeline_to_job(pipeline, is_stopped, in_foreground);
    if (job == NULL) {
        /* Restart the pipeline */
        kill(-pipeline->gid, SIGCONT);
        return -1;
    }

    /* We won't update `pipeline->is_running` to false
       because the pipeline is converted to job and we don't
       want to free the memory associated with the pipeline. */
    notify_job_status(job);
    return 0;
}


static Pipe_return_status
wait_for_pipeline(Pipeline *pipeline, bool in_subshell)
{
    bool has_sent_term_sig = false;

    while (true) {
        int status;
        pid_t pid = waitpid(-pipeline->gid, &status, WUNTRACED);

        if (pid == -1) {
            if (errno == ECHILD) {
                /* No more children to wait for */
                pipeline->is_running = false;
                break;
            }

            // TODO: Error handling
        }

        /* If process was suspended */
        if (WIFSTOPPED(status) == true) {
            if (!in_subshell) {
                handle_pipeline_suspension(pipeline);
                return PIPE_SUSPND;
            }
            /* If we're in subshell, the subshell will be
               stopped also and will be reported to parent shell */
        }

        /* If process was terminated by signal */
        else if (WIFSIGNALED(status) == true) {
            if (!in_subshell && !has_sent_term_sig) {
                int sig = WTERMSIG(status);
                handle_pipeline_termination(pipeline, sig, pid);
                has_sent_term_sig = true;
            }
        }

        /* If process exited successfully */
        else if (WIFEXITED(status)) {
           /* Find the child with the pid and update its return value */
            for (int i = 0; i < pipeline->process_count; i++) {
                if (pipeline->process[i]->pid == pid) {
                    pipeline->process[i]->return_val = WEXITSTATUS(status);
                }
            }
        }
    }

    if (has_sent_term_sig) {
        return PIPE_TERM;
    }

    return PIPE_EXIT;
}


#define WRITE_END 1
#define READ_END  0

static int
create_and_exec_child_process(Pipeline *pipeline, int index, int infile, int outfile,
                              bool is_last_proc, int pipefd[], bool in_subshell)
{
    pid_t pid = fork();

    switch (pid) {
        case -1:  /* fork fails */
            perror("Forking child process failed");
            return -1;

        /* Both parent and child processes need to update the process
           group of the child in order to eliminate race conditions. 
           This should not be done if child was spawned by subshell
           as both child and subshell should remain in same group. */

        case 0:     /* child process */
            if (!in_subshell) {
                pid = getpid();
                pid_t pgid = pipeline->gid;
                if (pgid == 0) {
                    /* If first member of group, make group leader */
                    pgid = pid;
                }
                setpgid(pid, pgid);
            }

            /* An extra file descriptor remained opened if not last process  */
            if (!is_last_proc) {
               close(pipefd[READ_END]);
            }
            launch_process(pipeline->process[index], infile, outfile);
            break;

        default:    /* parent process */
            if (!in_subshell) {
                /* Update child's process group */
                if (pipeline->gid == 0) {
                    pipeline->gid = pid;
                }
                setpgid(pid, pipeline->gid);
            }
            else {
                /* If in subshell, then gid of pipeline will be same as subshell's */
                if (pipeline->gid == 0) {
                    pipeline->gid = getpgrp();
                }
            }
            pipeline->process[index]->pid  = pid;
            pipeline->process[index]->pgid = pipeline->gid;
    }

    return 0;
}


#define CLEAN_UP_FDS(infile, outfile, pipefd)     \
        do {                                      \
            if (infile != STDIN_FILENO) {         \
                close(infile);                    \
            }                                     \
            if (outfile != STDOUT_FILENO) {       \
                close(outfile);                   \
            }                                     \
            infile = pipefd[READ_END];            \
        } while (false);                          \

/* Creates pipes, attaches processes to them and launches the processes */
static int
setup_and_launch_pipeline(Pipeline *pipeline, bool in_subshell)
{
    int pipefd[2];
    int infile = STDIN_FILENO;
    int outfile;

    /* Connect processes to pipeline */
    for (int i = 0; i < pipeline->process_count; i++) {
        bool is_last_proc = (i + 1 == pipeline->process_count);

        if (is_last_proc) {
            /* For last Process in pipeline, connect outfile to stdout */
            outfile = STDOUT_FILENO;
        } else {
            if (pipe(pipefd) < 0) {
                perror("Pipe creation failed");
                // TODO: Terminate all Processs in pipeline
                return -1;
            }
            outfile = pipefd[WRITE_END];
        }

        /* Launch the process */
        char **argv = pipeline->process[i]->argv;
        int    argc = pipeline->process[i]->argc;
        Builtin builtin;

        /* If process is builtin, then launch the builtin. Else launch process */
        if ((builtin = match_builtin(argv)) != BUILTIN_NONE) {
            int return_val = exec_builtin(builtin, argv, argc, infile, outfile);
            pipeline->process[i]->return_val = return_val;
        } else if (create_and_exec_child_process(pipeline, i, infile, outfile, is_last_proc, pipefd, in_subshell) == -1) {
            // TODO: Terminate all running Processs in pipeline
            return -1;
        }

        CLEAN_UP_FDS(infile, outfile, pipefd);
    }

    pipeline->is_running = true;
    return 0;
}

#undef WRITE_END
#undef READ_END
#undef CLEAN_UP_FDS


Pipe_return_status
launch_pipeline(Pipeline *pipeline, int *return_val, bool in_foreground, bool in_subshell)
{
    /* `in_foreground` will always be false for subshell started in background
       because whether the subshell should be in foreground or background must
       be decided by the job handler of parent shell. */

    if (setup_and_launch_pipeline(pipeline, in_subshell) == -1) {
        return -1;
    }

    if (in_foreground) {
        /* Put the pipeline as the foreground process (group) */
        tcsetpgrp(get_shell_terminal(), pipeline->gid);
    }

    if (!in_subshell) {
        block_sigchld();
    }
    /* Based on return status of waiting, set return values */
    Pipe_return_status return_stat = wait_for_pipeline(pipeline, in_subshell);
    switch (return_stat) {
        case PIPE_EXIT:  /* if pipeline exited successfully, return val of last process in pipeline */
            *return_val = pipeline->process[pipeline->process_count - 1]->return_val;
            break;

        /* The below two won't run if we're in a subshell executing in background */
        case PIPE_SUSPND: /* if pipeline suspended, return value is 0 (just like in bash) */
            *return_val = 0;
            break;

        case PIPE_TERM:  /* if pipeline was terminted, its reported as a failure */
            *return_val = 1;
            break;
    }

    if (!in_subshell) {
        unblock_sigchld();
    }

    if (in_foreground) {
        put_shell_in_foreground();
    }
    return return_stat;
}
