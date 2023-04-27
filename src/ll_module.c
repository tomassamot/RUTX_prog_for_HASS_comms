#include "ll_module.h"
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>

#define DELIMITER ","

struct ll_module* ll_module_create(char name[])
{
    struct ll_module *new_node = NULL;

    new_node = (struct ll_module*) malloc(sizeof(struct ll_module));
    strcpy(new_node->name, name);
    new_node->next = NULL;

    return new_node;
}
int ll_module_add(struct ll_module **list,  struct ll_module *new_node)
{
    struct ll_module *current_node = *list;

    if (current_node == NULL){
        *list = new_node;
        return 0;
    }

    while (current_node->next != NULL){
        current_node = current_node->next;
    }
        
    current_node->next = new_node;
    return 0;
}
int ll_module_insert(struct ll_module **list, struct ll_module *new_node, int index)
{
    struct ll_module *previous_node = NULL;
    struct ll_module *current_node = *list;
    int i = 0;
    if(i == index && i == 0){
        new_node->next = current_node;
        *list = new_node;
        return 0;
    }

    while (current_node != NULL || i != index){
        if(i == index){
            previous_node->next = new_node;
            new_node->next = current_node;
            return 0;
        }
        previous_node = current_node;
        current_node = current_node->next;
        i++;
    }
    return 1;
}
int ll_module_delete_at(struct ll_module **list, int index)
{
    struct ll_module *previous_node = NULL;
    struct ll_module *current_node = *list;
    int i = 0;

    if(i == index && i == 0){
        *list = current_node->next;
        free(current_node);
        return 0;
    }

    while(current_node != NULL){
        if(i == index){
            previous_node->next = current_node->next;
            free(current_node);
            return 0;
        }
        previous_node = current_node;
        current_node = current_node->next;
        i++;
    }
    syslog(LOG_WARNING, "Entry with given index not found.\n");
    return 1;
}
int ll_module_delete_all(struct ll_module **list)
{
    struct ll_module *current_node = *list;

    while(current_node != NULL){
        struct ll_module *next = current_node->next;
        current_node = NULL;
        free(current_node);
        current_node = next;
    }
    *list = NULL;
    return 0;
}
struct ll_module* ll_module_get(struct ll_module **list, int index)
{
    struct ll_module *current_node = *list;
    int i = 0;

    while (current_node != NULL){
        if(i == index){
            return current_node;
        }
        current_node = current_node->next;
        i++;
    }
    syslog(LOG_WARNING, "Entry with given index not found.\n");
}
void ll_module_deallocate(struct ll_module **list)
{
    struct ll_module *current_node = *list;
    while(current_node != NULL){
        struct ll_module *tmp = current_node->next;
        free(current_node);
        current_node = tmp;
    }
}