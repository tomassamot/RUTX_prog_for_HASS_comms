#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/limits.h>
#include <string.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include "luafuncs.h"
#include "constants.h"

static void call_lua_subscribe_script(lua_State *L, char* path, char *input_json);
static void call_lua_publish_script(lua_State *L, char* path, char* topic, struct mosquitto *mosq);
static bool check_if_mod_name_is_allowed(char name[], struct ll_module *allowed_mod_list);

int find_and_start_lua_subscribe_script(char topic_end[], char input_json[], struct ll_module *allowed_sub_modules)
{
    lua_State *L;
    DIR *d;
    struct dirent *dir;
    int ret;
    char script_name[50];

    L = luaL_newstate();
    luaL_openlibs(L);
    
    d = opendir(LUA_SUBSCRIBE_SCRIPTS_PATH);
    if(d == NULL){
        syslog(LOG_ERR, "Couldn't open directory in path: %s", LUA_SUBSCRIBE_SCRIPTS_PATH);
        return 1;
    }

    sprintf(script_name, "mint-%s.lua", topic_end);

    while ((dir = readdir(d)) != NULL) {
        if(strcmp(script_name, dir->d_name) != 0){
            // current file in directory and expected script_name do not match
            continue;
        }
        char name[30]; // file name with extension
        strcpy(name, dir->d_name);

        strtok(dir->d_name, EXTENSION_SEPERATOR); // removes extension

        if(check_if_mod_name_is_allowed(dir->d_name, allowed_sub_modules) == false)
            continue;

        char full_path[60];
        sprintf(full_path, "%s%s", LUA_SUBSCRIBE_SCRIPTS_PATH, script_name);
        
        call_lua_subscribe_script(L, full_path, input_json);
        break;
    }
    closedir(d);

    lua_close(L);

    return 0;
}

int find_and_start_lua_publish_scripts(struct mosquitto *mosq, struct ll_module *allowed_pub_modules)
{
    lua_State *L;
    DIR *d;
    struct dirent *dir;
    int ret;

    L = luaL_newstate();
    luaL_openlibs(L);
    
    d = opendir(LUA_PUBLISH_SCRIPTS_PATH);
    if(d == NULL){
        syslog(LOG_ERR, "Couldn't open directory in path: %s", LUA_PUBLISH_SCRIPTS_PATH);
        return 1;
    }

    syslog(LOG_INFO, "Calling Lua scripts");
    while ((dir = readdir(d)) != NULL) {
        char name[30]; // file name with extension
        strcpy(name, dir->d_name);

        strtok(dir->d_name, EXTENSION_SEPERATOR); // removes extension

        if(check_if_mod_name_is_allowed(dir->d_name, allowed_pub_modules) == false)
            continue;

        char temp[30];
        sprintf(temp, "%s", dir->d_name);

        char *prefix = strtok(temp, PREFIX_SEPERATOR); // gets "mint" prefix 
        char *name_without_prefix = strtok(NULL, PREFIX_SEPERATOR); // gets after prefix
        if(strcmp(prefix, "mint") == 0){
            char full_path[100];
            sprintf(full_path, "%s%s", LUA_PUBLISH_SCRIPTS_PATH, name);

            char topic[50];
            sprintf(topic, "home-assistant/mint/%s", name_without_prefix);

            call_lua_publish_script(L, full_path, topic, mosq);
        }

    }
    closedir(d);

    lua_close(L);

    return 0;
}

static bool check_if_mod_name_is_allowed(char name[], struct ll_module *allowed_mod_list)
{
    struct ll_module *mod = allowed_mod_list;
    while(mod != NULL){
        if(strcmp(mod->name, name) == 0)
            return true;
        mod = mod->next;
    }
    return false;
}

static void call_lua_subscribe_script(lua_State *L, char* path, char *input_json)
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

    lua_getglobal(L, "set_data");
    lua_pushstring(L, input_json);
    ret = lua_pcall(L, 1, 0, 0);
    if (ret){
        syslog(LOG_ERR, "Failed to run script: %s\n", lua_tostring(L, -1));
        return;
    }
    
    lua_getglobal(L, "destroy");
    ret = lua_pcall(L, 0, 0, 0);
    if (ret){
        syslog(LOG_ERR, "Failed to run script: %s\n", lua_tostring(L, -1));
        return;
    }
}

static void call_lua_publish_script(lua_State *L, char* path, char* topic, struct mosquitto *mosq)
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