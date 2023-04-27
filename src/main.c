#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <argp.h>

#include "argpfuncs.h"
#include "ll_module.h"
#include "becomedaemon.h"
#include "mosquittofuncs.h"
#include "luafuncs.h"

#include "constants.h"

static void handle_kill(int signum);
static void start_loop();

struct arguments arguments = { {[0 ... 19] = '\0'}, 0, {[0 ... 49] = '\0'}, {[0 ... 49] = '\0'}, 0, .pub_modules=NULL, .sub_modules=NULL};
struct mosquitto *mosq;
int program_is_killed = 0, rc = 0;
bool first_time = 1;

int main(int argc, char *argv[])
{
    int ret=0;

    start_parser(argc, argv, &arguments);

    printf("Starting...\n");

    openlog("prog_for_mint", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

    if(arguments.daemon != 0){
		if(0 != 0){
			syslog(LOG_INFO, "Trying to start daemon...");
			ret = become_daemon();
			if(ret){
				syslog(LOG_ERR, "Error starting daemon. Return code: %d", ret);
				closelog();
				return EXIT_FAILURE;
			}
			syslog(LOG_INFO, "Daemon started");
		}
	}

    signal(SIGKILL, handle_kill);
    signal(SIGTERM, handle_kill);
    signal(SIGINT, handle_kill);
    signal(SIGUSR1, handle_kill);

    mosquitto_lib_init();

    start_loop();
    
    mosquitto_lib_cleanup();
    return 0;
}
static void start_loop()
{
    int ret = 0;
    while(program_is_killed == 0){
        if(first_time == 1)
            first_time = 0;
        else
            sleep(5);

        syslog(LOG_INFO, "Trying to connect to MQTT broker...");
        ret = mosq_connect(&mosq, arguments.broker_address, arguments.broker_port, CLIENT_ID, arguments.username, arguments.password, arguments.sub_modules);
        if(ret != 0)
            continue;
        syslog(LOG_INFO, "Connected to MQTT broker");

        rc = 0;
        while(rc == 0 && program_is_killed == 0){
            rc += find_and_start_lua_publish_scripts(mosq, arguments.pub_modules);
            sleep(5);
        }

        syslog(LOG_INFO, "Disconnecting from MQTT broker");  
        mosq_disconnect(mosq);
    }
}
static void handle_kill(int signum)
{
    syslog(LOG_INFO, "Kill request detected");  
    program_is_killed = 1;
}