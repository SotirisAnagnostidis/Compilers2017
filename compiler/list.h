#ifndef __LIST_H__
#define __LIST_H__

#include "ast.h"


struct stack_node {
        ast element;
            struct stack_node *next;
};

typedef struct stack_node *my_stack;

extern my_list var_dimension;


extern void print_dimensions();
extern int remove_from_list(my_list *L);
extern my_list merge_lists(my_list l1, my_list l2);
extern void print_list(my_list l);
extern void my_insert(ast element, my_stack *S);
extern int my_remove(my_stack* S);

#endif
