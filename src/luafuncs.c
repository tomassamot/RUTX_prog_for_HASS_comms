#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/limits.h>
#include <string.h>

#include "luafuncs.h"

#define PREFIX_SEPERATOR "-"
#define EXTENSION_SEPERATOR "."

static void call_lua_script(lua_State *L, char* path, char* topic, struct mosquitto *mosq);

char LUA_SCRIPTS_DIR[] = "/usr/mylua/scripts/";
char LUA_OUTPUTS_DIR[] = "/usr/mylua/outputs/";


int start_lua_scripts(struct mosquitto *mosq)
{
    lua_State *L;
    DIR *d;
    struct dirent *dir;
    int ret;

    L = luaL_newstate();
    luaL_openlibs(L);
    
    d = opendir(LUA_SCRIPTS_DIR);
    if(d == NULL){
        syslog(LOG_ERR, "Couldn't open directory in path: %s", LUA_SCRIPTS_DIR);
        return 1;
    }

    syslog(LOG_INFO, "Calling Lua scripts");
    while ((dir = readdir(d)) != NULL) {
        char name[30];
        strcpy(name, dir->d_name);

        strtok(dir->d_name, EXTENSION_SEPERATOR); // removes extension

        char temp[30];
        sprintf(temp, "%s", dir->d_name);

        char *prefix = strtok(temp, PREFIX_SEPERATOR); // gets "mint" prefix 
        char *name_without_prefix = strtok(NULL, PREFIX_SEPERATOR); // gets after prefix
        if(strcmp(prefix, "mint") == 0){
            char full_path[60];
            sprintf(full_path, "%s%s", LUA_SCRIPTS_DIR, name);

            char topic[50];
            sprintf(topic, "home-assistant/mint/%s", name_without_prefix);

            call_lua_script(L, full_path, topic, mosq);
        }

    }
    closedir(d);

    lua_close(L);

    return 0;
}

static void call_lua_script(lua_State *L, char* path, char* topic, struct mosquitto *mosq)
{
    int ret;

    ret = luaL_loadfile(L, path);
    if (ret){
        syslog(LOG_ERR, "Couldn't load file: %s\n", lua_tostring(L, -1));
        return;
    }
    
    lua_pcall(L, 0, 0, 0);

    lua_getglobal(L, "init");
    ret = lua_pcall(L, 0, 1, 0);
    if (ret){
        syslog(LOG_ERR, "Failed to run script: %s\n", lua_tostring(L, -1));
        return;
    }

    lua_getglobal(L, "get_data");
    ret = lua_pcall(L, 0, 1, 0);
    if (ret){
        syslog(LOG_ERR, "Failed to run script: %s\n", lua_tostring(L, -1));
        return;
    }
    
    const char *result = lua_tostring(L, -1);
    mosq_loop(mosq, topic, result);
    
    lua_getglobal(L, "destroy");
    ret = lua_pcall(L, 0, 0, 0);
    if (ret){
        syslog(LOG_ERR, "Failed to run script: %s\n", lua_tostring(L, -1));
        return;
    }
}