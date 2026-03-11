#ifndef STACK_H_
#define STACK_H_


#include "ast.h"


typedef struct Stack
{
    Ast_node *node;
    struct Stack *prev;
} Stack;


int push_node_into_stack(Ast_node *node, Stack **stack);
Ast_node *pop_node_from_stack(Stack **stack);
void destroy_stack(Stack **stack);


#endif // STACK_H_