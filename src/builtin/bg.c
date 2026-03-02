#include "builtin_helper.h"
#include "job.h"

#include <stdbool.h>
#include <stdio.h>


int
builtin_bg(char **argv, int argc)
{
    Job *job_head = get_job_head();
    if (job_head == NULL) {
        fprintf(stderr, "shell: bg: No job running\n");
        return 1;
    }

    int job_number;
    if ((job_number = parse_args(argv, argc)) == -1) {
        fprintf(stderr, "shell: bg: Invalid args\n");
        return 1;
    }

    Job *job_node;
    if ((job_node = find_job_with_number(job_number)) == NULL) {
        fprintf(stderr, "shell: bg: No job with job number %d exists\n", job_number);
        return 1;
    }

    bool cont = true;
    put_job_in_background(job_node, cont);
    notify_job_status(job_node);
    return 0;
}
