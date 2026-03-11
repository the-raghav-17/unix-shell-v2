#include "stack.h"
#include "ast.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>


int
push_node_into_stack(Ast_node *node, Stack **stack)
{
    Stack *new = malloc(sizeof(*new));
    if (new == NULL) {
        perror("push_node_into_stack");
        return -1;
    }

    new->prev = *stack;
    new->node = node;
    *stack    = new;
    return 0;
}


Ast_node *
pop_node_from_stack(Stack **stack)
{
    assert(*stack != NULL);
    Ast_node *ast_node = (*stack)->node;
    Stack    *temp     = *stack;

    *stack = (*stack)->prev;
    free(temp);

    return ast_node;
}


void
destroy_stack(Stack **stack)
{
    while (*stack != NULL) {
        pop_node_from_stack(stack);
    }
}