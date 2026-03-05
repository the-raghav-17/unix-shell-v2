#include "executor.h"
#include "ast.h"
#include "job.h"
#include "list.h"
#include "exec_helper.h"
#include "pipeline.h"
#include "stack.h"
#include "sig.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>


static int traverse_ast(Ast_node *ast_root, bool in_foreground, bool in_subshell);
static void traverse_ast_in_subshell(Ast_node *ast_root, bool in_foreground);


static int
traverse_ast(Ast_node *ast_root, bool in_foreground, bool in_subshell)
{
    /*
        An AST will always have the structure like following:

                                (condition)
                                /         \
                               /           \
                            (condition)   [Pipeline]
                            /         \
                           /           \
                    (condition)   [Pipeline]
                    /         \
                   /           \
                [Pipeline]    [Pipeline]
    */

    Stack    *stack = NULL;
    Ast_node *node  = ast_root;

    while (node->type != PIPELINE) {
        /* Push nodes into stack and go to left child until 
           node of type `PIPELINE` is not found */

        if (push_node_into_stack(node, &stack) == -1) {
            destroy_stack(&stack);
            return -1;
        }
        node = node->left;
    }

    int return_val;
    Pipe_return_status return_stat 
        = launch_pipeline(node->pipeline, &return_val, in_foreground, in_subshell);

    /* If pipeline was launched as part of subshell, then termination and suspension
       should not affect tree traversal. This is not true if pipeline was launched
       by the parent shell. This behavior is similar to bash. */
    if (!in_subshell && (return_stat == PIPE_TERM || return_stat == PIPE_SUSPND)) {
        destroy_stack(&stack);
        return -1;
    }

    node->return_val = return_val;

    while (stack != NULL) {
        /* Start emptying the stack and execute the right
           child depending on the type of node */

        node = pop_node_from_stack(&stack);

        /* Current node type will always be `AND` or `OR` and its
           right child will always be of type `PIPELINE` */
        if (can_execute_right_pipeline(node)) {
            return_stat 
                = launch_pipeline(node->right->pipeline, &return_val, in_foreground, in_subshell);

            if (!in_subshell && (return_stat == PIPE_TERM || return_stat == PIPE_SUSPND)) {
                destroy_stack(&stack);
                return -1;
            }
            node->right->return_val = return_val;
            update_node_status(node);
        }
    }

    return 0;
}


static void
traverse_ast_in_subshell(Ast_node *ast_root, bool in_foreground)
{
    /* Create subshell */
    pid_t pid = fork();

    switch (pid) {
        case -1:
            perror("Subshell spawning failed");
            break;

        case 0:  /* subshell */
            bool in_subshell = true;

            /* Change group */
            pid = getpid();
            setpgid(pid, pid);

            reset_signal_disposition();
            disable_job_control();
            traverse_ast(ast_root, in_foreground, in_subshell);
            _exit(EXIT_SUCCESS); /* parent shell needs to reap it */

        default:
            /* Change group of subshell */
            setpgid(pid, pid);

            /* Parent shell; adds subshell to job list */
            bool is_stopped = false;
            Job *job = add_subshell_to_job(pid, is_stopped, in_foreground);
            if (job == NULL) {
                // TODO: Terminate subshell
                fprintf(stderr, "shell: Terminating all commands in subshell\n");
                return;
            }
            notify_job_status(job);
    }
}


void
execute(List_node *head)
{
    /* Traverse each node of the list */
    for (List_node *node = head; node != NULL; node = node->next) {
        /* If node is not in foreground, it means 
        background execution in subshell */
        bool in_foreground = node->is_foreground;
        bool in_subshell   = !in_foreground;

        if (in_subshell) {
            traverse_ast_in_subshell(node->ast_root, in_foreground);
        }

        else {
            if (traverse_ast(node->ast_root, in_foreground, in_subshell) == -1) {
                return ;
            }
        }
    }
}
