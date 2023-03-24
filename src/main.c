#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "becomedaemon.h"
#include "mosquittofuncs.h"
#include "ubusfuncs.h"


void handle_kill(int signum);
static void board_cb(struct ubus_request *req, int type, struct blob_attr *msg);


//char *address = "tcp://192.168.10.103:1883";
char *address = "192.168.10.103";
int port = 1883;
char *client_id = "mint_caller_prog";
char *me_to_hass_topic = "home-assistant/mint/ram";
//char *hass_to_me_topic = "";
char *username = "broker";
char *password = "brokerpass";

int program_is_killed = 0, rc = 0;
struct mosquitto *mosq_client;

int main(int argc, char *argv[])
{
    int ret=0, is_daemon = 0;
    struct ubus_context *ubus_context;
    uint32_t id;

    printf("Starting...\n");

    openlog("device_inhibitor", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

    // ret = become_daemon();
    // if(ret){
    //     syslog(LOG_ERR, "Error starting daemon");
    //     closelog();
    //     return EXIT_FAILURE;
    // }
    // syslog(LOG_INFO, "Daemon started");
    
    signal(SIGKILL, handle_kill);
    signal(SIGTERM, handle_kill);
    signal(SIGINT, handle_kill);
    signal(SIGUSR1, handle_kill);

    ret = connect_to_ubus(&ubus_context);
    if(ret != 0)
        exit(EXIT_FAILURE);

    ret = mosq_connect(&mosq_client, address, port, client_id, username, password);
    if(ret != 0)
        exit(EXIT_FAILURE);

    ubus_lookup_id(ubus_context, "system", &id);
    rc = 0;
    while(rc == 0 && program_is_killed == 0){
        sleep(2);
        ret = ubus_invoke(ubus_context, id, "info", NULL, board_cb, NULL, 3000);
        if(ret != 0)
            syslog(LOG_ERR, "Failed to invoke ubus");
    }

    disconnect_from_ubus(ubus_context);
    mosq_disconnect(mosq_client);
    
    return 0;
}
static void board_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
    char message[200];
	//struct blob_buf *buf = (struct blob_buf *)req->priv;
	struct blob_attr *tb[__INFO_MAX];
	struct blob_attr *memory[__MEMORY_MAX];

	blobmsg_parse(info_policy, __INFO_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[MEMORY_DATA]) {
		syslog(LOG_WARNING, "No memory data received");
        rc--;
		return;
	}

	blobmsg_parse(memory_policy, __MEMORY_MAX, memory, blobmsg_data(tb[MEMORY_DATA]), blobmsg_data_len(tb[MEMORY_DATA]));

    sprintf(message, "{\"free_ram\":%lld,\"total_ram\":%lld}", blobmsg_get_u64(memory[FREE_MEMORY]), blobmsg_get_u64(memory[TOTAL_MEMORY]));
    int ret = mosq_loop(mosq_client, me_to_hass_topic, message);
    if(ret != 0){
        rc+=1;
    }
}
void handle_kill(int signum)
{
    program_is_killed = 1;
}