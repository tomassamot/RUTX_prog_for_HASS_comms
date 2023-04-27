#ifndef LUAFUNCS_H
#define LUAFUNCS_H

#include "mosquittofuncs.h"
#include "ll_module.h"

int find_and_start_lua_subscribe_script(char topic_end[], char input_json[], struct ll_module *sub_modules);
int find_and_start_lua_publish_scripts(struct mosquitto *mosq, struct ll_module *pub_modules);

#endif