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

// #define PREFIX_SEPERATOR "-"
// #define EXTENSION_SEPERATOR "."

static void call_lua_subscribe_script(lua_State *L, char* path, char *input_json);
static void call_lua_publish_script(lua_State *L, char* path, char* topic, struct mosquitto *mosq);


int find_and_start_lua_subscribe_script(char topic_end[], char input_json[])
{
    lua_State *L;
    DIR *d;
    struct dirent *dir;
    int ret;
    char script_name[50];

    printf("hello1\n");
    L = luaL_newstate();
    luaL_openlibs(L);
    
    printf("hello2\n");
    d = opendir(LUA_SUBSCRIBE_SCRIPTS_PATH);
    if(d == NULL){
        syslog(LOG_ERR, "Couldn't open directory in path: %s", LUA_SUBSCRIBE_SCRIPTS_PATH);
        return 1;
    }

    printf("hello3\n");
    sprintf(script_name, "mint-%s.lua", topic_end);

    while ((dir = readdir(d)) != NULL) {
        printf("hello4\n");
        if(strcmp(script_name, dir->d_name) == 0){
            printf("hello4.1\n");
            char full_path[60];
            sprintf(full_path, "%s%s", LUA_SUBSCRIBE_SCRIPTS_PATH, script_name);

            printf("hello4.2\n");
            call_lua_subscribe_script(L, full_path, input_json);
            printf("hello4.3\n");
            break;
        }
        printf("hello5\n");

    }
    closedir(d);

    // char ban_dev_path[] = "/usr/mylua/scripts/subscribe/mint-ban.lua";
    // char input_json[] = "{\"addr\": \"4A:47:ED:BC:3F:B9\", \"reason\": 1, \"deauth\": false, \"ban_time\": 5}";
    // call_lua_subscribe_script(L, ban_dev_path, input_json);

    lua_close(L);

    return 0;

    return 0;
}

int find_and_start_lua_publish_scripts(struct mosquitto *mosq)
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
        char name[30];
        strcpy(name, dir->d_name);

        strtok(dir->d_name, EXTENSION_SEPERATOR); // removes extension

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

    // char ban_dev_path[] = "/usr/mylua/scripts/subscribe/mint-ban.lua";
    // char input_json[] = "{\"addr\": \"4A:47:ED:BC:3F:B9\", \"reason\": 1, \"deauth\": false, \"ban_time\": 5}";
    // call_lua_subscribe_script(L, ban_dev_path, input_json);

    lua_close(L);

    return 0;
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