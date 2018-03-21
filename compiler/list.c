#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "list.h"

void print_dimensions(){
    printf("======== HERE  ARE THE DIMENSIONS ======== \n");
    list temp = var_dimension;
    while (temp!=NULL){
        printf(" -- %d \n" , temp->line);
        temp = temp->next;
    }
    printf("======== END OF DIMENSIONS ========\n");
}

int remove_from_list(my_list *L){
    if (*L == NULL)
        return -1;
    int return_value = (*L)->line;
    my_list temp = *L;
    *L = (*L)->next;
    free(temp);
    return return_value;
}



void print_list(my_list l){
    printf("========== HERE  IS THE LIST ==========\n");
    my_list temp = l;
    while (temp!=NULL){
        printf(" ||  %5d                       ||\n" , temp->line);
        temp = temp->next;
    }
    printf("========== END OF LIST ==========\n");
}






