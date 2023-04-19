#ifndef LUAFUNCS_H
#define LUAFUNCS_H

#include "mosquittofuncs.h"

int start_lua_scripts(struct mosquitto *mosq);
int find_and_start_lua_subscribe_script(char topic_end[], char input[]);
int find_and_start_lua_publish_scripts(struct mosquitto *mosq);

#endif