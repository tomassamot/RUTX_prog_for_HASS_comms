#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <argp.h>

#include "argpfuncs.h"
#include "becomedaemon.h"
#include "mosquittofuncs.h"
#include "ubusfuncs.h"
#include "luafuncs.h"

static void handle_kill(int signum);
static int send_ram_information(struct ubus_context *ubus_context);
static void board_cb(struct ubus_request *req, int type, struct blob_attr *msg);
static int send_wireless_information(struct ubus_context *ubus_context);
static void process_wireless_info(struct ubus_request *req, int type, struct blob_attr *msg);
static void start_loop();

struct arguments arguments = { {[0 ... 19] = '\0'}, 0, {[0 ... 49] = '\0'}, {[0 ... 49] = '\0'}, 0 };
struct mosquitto *mosq;
int program_is_killed = 0, rc = 0;
bool first_time = 1;

const char *CLIENT_ID = "mint_caller_prog";
const char *RAM_TOPIC = "home-assistant/mint/ram";
const char *WIRELESS_TOPIC = "home-assistant/mint/wireless";

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
    struct ubus_context *ubus_context;
    int ret = 0;
    while(program_is_killed == 0){
        if(first_time == 1)
            first_time = 0;
        else
            sleep(5);

        syslog(LOG_INFO, "Trying to connect to ubus...");
        ret = connect_to_ubus(&ubus_context);
        if(ret != 0)
             continue;
        syslog(LOG_INFO, "Connected to ubus");  

        syslog(LOG_INFO, "Trying to connect to MQTT broker...");
        ret = mosq_connect(&mosq, ubus_context, arguments.broker_address, arguments.broker_port, CLIENT_ID, arguments.username, arguments.password);
        if(ret != 0)
            continue;
        syslog(LOG_INFO, "Connected to MQTT broker");

        rc = 0;
        while(rc == 0 && program_is_killed == 0){
			// rc += send_ram_information(ubus_context);
            // rc += send_wireless_information(ubus_context);
            rc += start_lua_scripts(mosq);
            sleep(5);
        }

        syslog(LOG_INFO, "Disconnecting from ubus");  
        disconnect_from_ubus(ubus_context);
        syslog(LOG_INFO, "Disconnecting from MQTT broker");  
        mosq_disconnect(mosq);
    }
}
static int send_ram_information(struct ubus_context *ubus_context)
{
    uint32_t id;

    ubus_lookup_id(ubus_context, "system", &id);
    int ret = ubus_invoke(ubus_context, id, "info", NULL, board_cb, NULL, 3000);
    if(ret != 0)
        syslog(LOG_ERR, "Failed to invoke ubus. Return code: %d",ret);
    return ret;
}
static void board_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    char message[200];
	struct blob_attr *tb[__INFO_MAX];
	struct blob_attr *memory[__MEMORY_MAX];

	blobmsg_parse(info_policy, __INFO_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[MEMORY_DATA]) {
		syslog(LOG_WARNING, "No memory data received");
        rc--;
		return;
	}
	blobmsg_parse(memory_policy, __MEMORY_MAX, memory, blobmsg_data(tb[MEMORY_DATA]), blobmsg_data_len(tb[MEMORY_DATA]));

    sprintf(message, "{\"free_ram\":%lld,\"total_ram\":%lld, \"available_ram\":%lld}", blobmsg_get_u64(memory[FREE_MEMORY]), blobmsg_get_u64(memory[TOTAL_MEMORY]), blobmsg_get_u64(memory[AVAILABLE_MEMORY]));
    int ret = mosq_loop(mosq, RAM_TOPIC, message);
    if(ret != 0)
        rc+=1;
}
static int send_wireless_information(struct ubus_context *ubus_context)
{
    uint32_t id;

    ubus_lookup_id(ubus_context, "network.wireless", &id);
    int ret = ubus_invoke(ubus_context, id, "status", NULL, process_wireless_info, NULL, 3000);
    if(ret != 0)
        syslog(LOG_ERR, "Failed to invoke ubus. Return code: %d",ret);
    return ret;
}
static void process_wireless_info(struct ubus_request *req, int type, struct blob_attr *msg)
{
    struct blob_attr *status[2];
    int ret = 0;
    
    static const struct blobmsg_policy status_policy[2] = {
        [0] = { .name = "radio0", .type = BLOBMSG_TYPE_TABLE },
        [1] = { .name = "radio1", .type = BLOBMSG_TYPE_TABLE },
    };

    blobmsg_parse(status_policy, 2, status, blob_data(msg), blob_len(msg));

    char *radio0 = get_radio_info(status, 0);
    char *radio1 = get_radio_info(status, 1);

    ret = mosq_loop(mosq, WIRELESS_TOPIC, radio0);
    if(ret != 0)
        rc+=1;
    ret = mosq_loop(mosq, WIRELESS_TOPIC, radio1);
    if(ret != 0)
        rc+=1;

    free(radio0);
    free(radio1);
}
static void handle_kill(int signum)
{
    syslog(LOG_INFO, "Kill request detected");  
    program_is_killed = 1;
}