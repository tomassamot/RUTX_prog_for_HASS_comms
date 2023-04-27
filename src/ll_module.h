#ifndef LL_MODULE_H
#define LL_MODULE_H

struct ll_module{
	char name[50];
	struct ll_module *next;
};

struct ll_module* ll_module_create(char name[]);
int ll_module_add(struct ll_module **list,  struct ll_module *new_node);
int ll_module_insert(struct ll_module **list, struct ll_module *new_node, int index);
int ll_module_delete_at(struct ll_module **list, int index);
int ll_module_delete_all(struct ll_module **list);
struct ll_module* ll_module_get(struct ll_module **list, int index);
void ll_module_deallocate(struct ll_module **list);

#endif