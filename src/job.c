#include "job.h"
#include "pipeline.h"
#include "process.h"
#include "shell.h"

#include <bits/types/idtype_t.h>
#include <bits/types/siginfo_t.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>


#define STOP_JOB(job) (kill(-job->gid, SIGSTP))
#define CONT_JOB(job) (kill(-job->gid, SIGCONT))


static int get_job_number(Job *job_node);
static void add_node_to_job_list(Job *job_node);
static void destroy_job_obj(Job *job);
static void terminate_job(Job *job);
static void wait_for_job(Job *job);



/* Jobs will be stored in a linked list, of
   which job_head is the head */
static Job *job_head = NULL;


Job *
get_job_head(void)
{
    return job_head;
}


static int
get_job_number(Job *job_node)
{
    int job_number = 1;
    Job *temp      = job_head;

    while (temp != job_node) {
        temp        = temp->next;
        job_number += 1;
    }
    return job_number;
}


Job *
find_job_with_number(int job_number)
{
    Job *job_node = job_head;
    for (int i = 1; i < job_number && job_node != NULL; i++) {
        job_node = job_node->next;
    }
    return job_node;
}


/* Add the job_node to end of the job list */
static void
add_node_to_job_list(Job *job_node)
{
    if (job_head == NULL) {
        job_head = job_node;
        return;
    }

    Job *ptr = job_head;
    while (ptr->next != NULL) {
        ptr = ptr->next;
    }
    ptr->next = job_node;
}


static void
destroy_job_obj(Job *job)
{
    /* Destroy the processes associated with job.*/
    if (!job->is_subshell) {
        for (int i = 0; i < job->process_count; i++) {
            destroy_process_obj(job->process[i]);
        }
    }

    free(job->process);

    /* Find the job node in the job list and destroy it */
    Job *prev = NULL;
    Job *ptr  = job_head;

    /* Traverse the list and find job node */
    while (ptr != job && ptr != NULL) {
        prev = ptr;
        ptr  = ptr->next;
    }

    if (prev == NULL) {
        /* Head is the target */
        job_head = ptr->next;
    } else {
        prev->next = ptr->next;
    }

    free(ptr);
}


Job *
add_pipeline_to_job(Pipeline *pipeline, bool is_stopped, bool in_foreground)
{
    Job *job_node = malloc(sizeof(*job_node));
    if (job_node == NULL) {
        perror("Creating job out of pipeline failed");
        return NULL;
    }

    job_node->next          = NULL;
    job_node->gid           = pipeline->gid;
    job_node->process       = pipeline->process;
    job_node->process_count = pipeline->process_count;
    job_node->is_stopped    = is_stopped;
    job_node->in_foreground = in_foreground;
    job_node->is_completed  = false;

    /* Save terminal settings */
    tcgetattr(get_shell_terminal(), &job_node->tmodes);

    add_node_to_job_list(job_node);
    return job_node;
}


Job *
add_subshell_to_job(pid_t gid, bool is_stopped, bool in_foreground)
{
    Job *job_node = malloc(sizeof(*job_node));
    if (job_node == NULL) {
        perror("Creating job out of subshell failed");
        return NULL;
    }

    job_node->is_subshell   = true;
    job_node->gid           = gid;  /* pid of subshell */
    job_node->in_foreground = in_foreground;
    job_node->is_stopped    = is_stopped;
    
    job_node->process       = NULL;
    job_node->process_count = 1;    /* the subshell */
    job_node->is_completed  = false;
    job_node->next          = NULL;
    
    /* Save terminal settings */
    tcgetattr(get_shell_terminal(), &job_node->tmodes);

    add_node_to_job_list(job_node);
    return job_node;
}


static void
terminate_job(Job *job)
{
    kill(-job->gid, SIGINT);

    /* Reap children */
    siginfo_t infop;
    for (int i = 0; i < job->process_count; i++) {
        waitid(P_PGID, job->gid, &infop, WEXITED | WNOHANG);
    }
}


void
terminate_all_jobs(void)
{
    for (Job *job = job_head; job != NULL; job = job->next) {
        terminate_job(job);
        /* destroy_job_obj(job); */
    }
}


void
notify_job_status(Job *job)
{
    printf("\n[%d] %s     *job processes go here*\n", get_job_number(job), 
                job->is_stopped ? "Stopped" : "Running");
}


void
put_job_in_background(Job *job, bool cont)
{
    job->in_foreground = false;
    if (cont == true) {
       CONT_JOB(job);
       job->is_stopped = false;
    }
}


static void
wait_for_job(Job *job)
{
    siginfo_t infop;

    for (int i = 0; i < job->process_count; i++) {
        waitid(P_PGID, job->gid, &infop, WEXITED | WSTOPPED);

        int si_code = infop.si_code;
        bool has_sent_kill_sig = false;

        switch (si_code) {
            case CLD_EXITED:
                /* Do nothing as we've catched the return
                   status of child process and just needd
                   need to destroy the associated process obj */
                break;

            case CLD_STOPPED:
                /* Make sure every process in the job is stopped */
                kill(job->gid, SIGSTOP);
                
                /* Put back the job in background */
                job->is_stopped = true;
                bool cont       = false;
                put_job_in_background(job, cont);
                notify_job_status(job);
                return;

            case CLD_KILLED:
                /* Send terminate signal to every
                   process in the group but just once */
                if (!has_sent_kill_sig) {
                    int sig = infop.si_status;
                    kill(-job->gid, sig);
                    has_sent_kill_sig = true;
                }
                printf("\n");
                break;
        }
    }

    job->is_completed = true;
    destroy_job_obj(job);
}


void
put_job_in_foreground(Job *job, bool cont)
{
    // BUG TODO: Trying to put a subshell into foreground when
    //  its stopped leads to a bug

    if (job == NULL) {
        return;
    }
    /* Put the job in foreground and make sure its running */
    tcsetpgrp(get_shell_terminal(), job->gid);
    tcsetattr(get_shell_terminal(), TCSANOW, &job->tmodes);
    if (cont == true) {
        CONT_JOB(job);
        job->is_stopped = false;
    }

    job->in_foreground = true;

    wait_for_job(job);
    put_shell_in_foreground();
}
