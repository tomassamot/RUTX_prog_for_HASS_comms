#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "ubusfuncs.h"

static void get_connected_devices(char buffer[], char *ifname);

int rcc = 0;

int connect_to_ubus(struct ubus_context **ctx)
{
    *ctx = ubus_connect(NULL);
    if(ctx == NULL)
        return -1;
    return 0;
}
void disconnect_from_ubus(struct ubus_context *ctx)
{
    ubus_free(ctx);
}
int set_new_static_ip(struct ubus_context *ctx, char *ip)
{
	struct blob_buf b = {};
	void *o2;
	uint32_t id;

    rcc = 0;

	blob_buf_init(&b, 0);
	
	blobmsg_add_string(&b, "config", "network");
	blobmsg_add_string(&b, "section", "lan");
	o2 = blobmsg_open_table(&b, "values");
	blobmsg_add_string(&b, "ipaddr", ip);
	blobmsg_close_table(&b, o2);

	if (ubus_lookup_id(ctx, "uci", &id)) {
		syslog(LOG_ERR, "Failed to find id of command 'uci'");
        rcc=1;
	}
	if (ubus_invoke(ctx, id, "set", b.head, NULL, NULL, 3000)) {
		syslog(LOG_ERR, "Failed to invoke 'set' of 'uci'");
        rcc=2;
	}
	blob_buf_free(&b);

	return rcc;
}
int commit_uci_changes(struct ubus_context *ctx, const char *config)
{
	struct blob_buf c = {};
	uint32_t id;

    rcc = 0;
    
    blob_buf_init(&c, 0);

    if(config != NULL)
	    blobmsg_add_string(&c, "config", "network");

	if (ubus_lookup_id(ctx, "uci", &id)) {
		syslog(LOG_ERR, "Failed to find id of command 'uci'");
        rcc=1;
	}
	if (ubus_invoke(ctx, id, "commit", c.head, NULL, NULL, 3000)) {
		syslog(LOG_ERR, "Failed to invoke 'commit' of 'uci'");
		rcc=2;
	}

	blob_buf_free(&c);

	return 0;
}
char* get_radio_info(struct blob_attr *status[], int i)
{
    struct blob_attr *radio[4];
    struct blob_attr *config[3];

    char radio_name[7];
    sprintf(radio_name, "radio%d", i);
    
    blobmsg_parse(radio_policy, 4, radio, blobmsg_data(status[i]), blobmsg_data_len(status[i]));

    blobmsg_parse(config_policy, 3, config, blobmsg_data(radio[2]), blobmsg_data_len(radio[2]));

    struct blob_attr *curr;
    size_t rem = 0;
    char conn_devices[512];
    int j = 0;

    char *all_interfaces = NULL;

    blobmsg_for_each_attr(curr, radio[3],rem){
        struct blob_attr *interface[3];
        struct blob_attr *ifconfig[2];
        char ifinfo[1024];
        if(j == 0){
            all_interfaces = (char *) malloc(2*1024*sizeof(char)); // initially 2 interfaces of size 1024 chars
            strcpy(all_interfaces, "");
        }
        else if(j % 2 == 0)
            all_interfaces = (char *) realloc(all_interfaces, 2*j*1024*sizeof(char));
        
        if(all_interfaces == NULL){
            syslog(LOG_ERR, "Failed to allocate memory for interfaces");
            break;
        }
        blobmsg_parse(interface_policy, 2, interface, blobmsg_data(curr), blobmsg_data_len(curr));

        memset(conn_devices, 0, sizeof conn_devices);
        get_connected_devices(conn_devices, blobmsg_get_string(interface[0]));

        blobmsg_parse(ifconfig_policy, 2, ifconfig, blobmsg_data(interface[1]), blobmsg_data_len(interface[1]));

        if(j == 0)
            sprintf(ifinfo, "{\"ifname\": \"%s\",\"conn_devices\": \"%s\",\"ifconfig.ssid\": \"%s\",\"ifconfig.wifi_id\": \"%s\"}", blobmsg_get_string(interface[0]), conn_devices, blobmsg_get_string(ifconfig[0]), blobmsg_get_string(ifconfig[1]));
        else
            sprintf(ifinfo, ",{\"ifname\": \"%s\",\"conn_devices\": \"%s\",\"ifconfig.ssid\": \"%s\",\"ifconfig.wifi_id\": \"%s\"}", blobmsg_get_string(interface[0]), conn_devices, blobmsg_get_string(ifconfig[0]), blobmsg_get_string(ifconfig[1]));

        strcat(all_interfaces, ifinfo);

        j++;
    }
    char *result = (char *) malloc(1024*sizeof(char)+j*1024*sizeof(char));
    sprintf(result, "{\"radio_name\": \"%s\", \"up\": %d, \"channel\": \"%s\", \"hwmode\": \"%s\", \"htmode\": \"%s\", \"all_interfaces\": [%s]}", radio_name, blobmsg_get_bool(radio[0]), blobmsg_get_string(config[0]), blobmsg_get_string(config[1]), blobmsg_get_string(config[2]), all_interfaces);

    free(all_interfaces);
    return result;
}
#define DELIMITER " "
static void get_connected_devices(char buffer[], char *ifname)
{
    char cmd_buf[512];
    char command[100];
    
    sprintf(command, "iwinfo %s assoclist", ifname);
    FILE *cmd = popen(command, "r");
    int i = 0;
    while (fgets(cmd_buf, sizeof(cmd_buf), cmd) != 0) {
        if(i % 5 == 0){
            char *device_id = strtok(cmd_buf, DELIMITER);
            if(device_id != NULL){
                strcat(buffer, device_id);
                strcat(buffer, " ");
            }
        }
        i++;
    }
    pclose(cmd);
}