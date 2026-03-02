/*
    A job is process/pipeline that the shell manages asynchronously. Lets say
    there's a pipeline that is being executed in the foreground. If the
    user presses C-z in the terminal, it sends a suspend signal to all
    processes of that foreground pipeline. Once suspended, the pipeline
    is promoted to a job. Meaning, the shell will refer to that pipeline 
    (or the process group) as a job. The shell will know  all the processes 
    in the job, their PIDs, their arguments and all.
    
    This makes it easier to send signals to the process group refered by
    the job and collect the return statuses of the processes in that job.

    But there's a special case if a Process is invoked in background.
    Ex - `cmd1 | cmd2 && cmd3 | cmd4 &`. Because of the `&` at the
    end, all the Processs in this list will execute in the background.
    For that, the shell will spawn a sub-shell, that traverses the ast
    and launches Processs associated with this list.
    In that case, the subshell will be the sole member of the job that
    the parent shell manages, as its the only process that the parent
    shell spawned directly; the processes launched by the subshell are
    not direct descendents of the parent shell and will be managed by
    the subshell itself.
*/


#ifndef JOB_H_
#define JOB_H_


#include "process.h"
#include "pipeline.h"
#include "shell.h"

#include <sys/types.h>
#include <termios.h>


typedef struct Job
{
    //TODO: Add a subshell field when dealing with subshells
    bool        is_subshell;

    struct Job *next;
    pid_t       gid;           /* process group managed by the job */

    Process   **process;       /* processes in the job/process group */
    int         process_count; /* number of processes in the job */

    bool        in_foreground; /* true if job is running in foreground */
    bool        is_stopped;    /* true if job is currently suspended */
    bool        is_completed;  /* true if every process in job is completed */

    struct termios tmodes;     /* terminal settings of the job */
} Job;


/* Converts a pipeline to equivalent job object. 
   The processes in the pipeline MUST NOT be freed
   by the caller as its managed by the job handler. */
Job *add_pipeline_to_job(Pipeline *pipeline, bool is_stopped, bool in_foreground);

Job *add_subshell_to_job(pid_t gid, bool is_stopped, bool in_foreground);
void notify_job_status(Job *job);
Job *get_job_head(void);
void put_job_in_foreground(Job *job, bool cont);
void put_job_in_background(Job *job, bool cont);
Job *find_job_with_number(int job_number);
void terminate_all_jobs(void);


#endif // JOB_H_
