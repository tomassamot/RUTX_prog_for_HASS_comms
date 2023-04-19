#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <dirent.h>

#include "mosquittofuncs.h"
#include "constants.h"
#include "luafuncs.h"

static void on_connect(struct mosquitto *mosq, void *obj, int reason_code);
static void on_disconnect(struct mosquitto *mosq, void *obj, int reason_maybe);
static void on_publish(struct mosquitto *mosq, void *obj, int mid);
static void subscribe_for_each_lua_subscribe_script(struct mosquitto *mosq);
static void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);
static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);

// const char *NEW_IP_TOPIC = "home-assistant/mint/new_ip";
// const char *INIT_TOPIC = "home-assistant/mint/init";
// const char *EMOTION_REQUEST_TOPIC = "home-assistant/mint/emotion_req";
// const char *EMOTION_RESPONSE_TOPIC = "home-assistant/mint/emotion_res";
// const char *BAN_DEVICE_TOPIC = "home-assistant/mint/ban_device";

int ret = 0;
bool is_connected = 0;
char *current_emotion = "das";
struct ubus_context *ubus_context = NULL;

int mosq_connect(struct mosquitto **mosq, struct ubus_context *ubus_ctx, char *broker, int port, const char *client_id, char *username, char *password)
{
	ubus_context = ubus_ctx;
	*mosq = mosquitto_new(NULL, true, NULL);
	if(*mosq == NULL){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	mosquitto_connect_callback_set(*mosq, on_connect);
	mosquitto_disconnect_callback_set(*mosq, on_disconnect);
	mosquitto_publish_callback_set(*mosq, on_publish);
	mosquitto_subscribe_callback_set(*mosq, on_subscribe);
	mosquitto_message_callback_set(*mosq, on_message);
    mosquitto_username_pw_set(*mosq, username, password);

	ret = mosquitto_connect(*mosq, broker, port, 60);
	if(ret != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(*mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(ret));
		return 1;
	}

	ret = mosquitto_loop_start(*mosq);
	if(ret != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(*mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(ret));
		return 1;
	}
    return ret;
}
int mosq_loop(struct mosquitto *mosq, const char *topic, char *payload)
{
	if(is_connected == 0)
		return 1;
	ret = mosquitto_publish(mosq, NULL, topic, strlen(payload), payload, 0, false);
	if(ret != MOSQ_ERR_SUCCESS){
		syslog(LOG_ERR, "Error publishing: %s", mosquitto_strerror(ret));
		return ret;
	}
	return 0;
}
int mosq_disconnect(struct mosquitto *mosq)
{
	ubus_context = NULL;
	mosquitto_disconnect(mosq);
	mosquitto_loop_stop(mosq, false);
	mosquitto_destroy(mosq);
	return 0;
}

static void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	if(reason_code != 0){
		syslog(LOG_ERR, "Failed to connect. %s", mosquitto_strerror(reason_code));
		mosquitto_disconnect(mosq);
		return;
	}
	is_connected = 1;
	syslog(LOG_INFO, "Trying to subscribe...");
	ret = mosquitto_subscribe(mosq, NULL, NEW_IP_TOPIC, 1);
	if(ret != MOSQ_ERR_SUCCESS)
		syslog(LOG_ERR, "Error subscribing: %s", mosquitto_strerror(ret));
	ret = mosquitto_subscribe(mosq, NULL, EMOTION_REQUEST_TOPIC, 1);
	if(ret != MOSQ_ERR_SUCCESS)
		syslog(LOG_ERR, "Error subscribing: %s", mosquitto_strerror(ret));

	// ret = mosquitto_subscribe(mosq, NULL, BAN_DEVICE_TOPIC, 1);
	// if(ret != MOSQ_ERR_SUCCESS)
	// 	syslog(LOG_ERR, "Error subscribing: %s", mosquitto_strerror(ret));

	subscribe_for_each_lua_subscribe_script(mosq);

}
static void subscribe_for_each_lua_subscribe_script(struct mosquitto *mosq)
{
    DIR *d;
    struct dirent *dir;
    int ret;

    d = opendir(LUA_SUBSCRIBE_SCRIPTS_PATH);
    if(d == NULL){
        syslog(LOG_ERR, "Couldn't open directory in path: %s", LUA_SUBSCRIBE_SCRIPTS_PATH);
        return;
    }

    syslog(LOG_INFO, "Subscribing to topics for each Lua subscribe script");
    while ((dir = readdir(d)) != NULL) {
        char name[30];
        strcpy(name, dir->d_name);

        strtok(dir->d_name, EXTENSION_SEPERATOR); // removes extension

        char temp[30];
        sprintf(temp, "%s", dir->d_name);

        char *prefix = strtok(temp, PREFIX_SEPERATOR); // gets "mint" prefix 
        char *name_without_prefix = strtok(NULL, PREFIX_SEPERATOR); // gets after prefix
        if(strcmp(prefix, "mint") == 0){
            char topic[50];
            sprintf(topic, "home-assistant/mint/%s", name_without_prefix);

			syslog(LOG_INFO, "Subscribing to topic: %s", topic);
            ret = mosquitto_subscribe(mosq, NULL, topic, 1);
			if(ret != MOSQ_ERR_SUCCESS)
				syslog(LOG_ERR, "Error subscribing: %s", mosquitto_strerror(ret));
        }

    }
    closedir(d);

    // char file_name[] = "ban";
    // char input_json[] = "{\"addr\": \"4A:47:ED:BC:3F:B9\", \"reason\": 1, \"deauth\": false, \"ban_time\": 5000, \"ifname\": \"wlan0\"}";
    // find_and_start_lua_subscribe_script(file_name, input_json);
}
static void on_disconnect(struct mosquitto *mosq, void *obj, int reason_maybe)
{
	is_connected = 0;
}

static void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
	syslog(LOG_INFO, "Publishing message to HA");
}

static void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	bool have_subscription = false;

	for(int i=0; i<qos_count; i++){
		syslog(LOG_INFO, "on_subscribe: %d:granted qos = %d", i, granted_qos[i]);
		if(granted_qos[i] <= 2){
			have_subscription = true;
		}
	}
	if(have_subscription == false){
		syslog(LOG_ERR, "Error: All subscriptions rejected");
		mosquitto_disconnect(mosq);
	}
}

static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	syslog(LOG_INFO, "on_message triggered");
	syslog(LOG_INFO, "%s %d %s\n", msg->topic, msg->qos, (char *)msg->payload);
		if(strcmp(msg->topic, NEW_IP_TOPIC) == 0){
			syslog(LOG_INFO, "Received request to change IP");
			syslog(LOG_INFO, "msg: %s", msg->payload);

			ret = set_new_static_ip(ubus_context, msg->payload);
			if(ret == 0)
				commit_uci_changes(ubus_context, "network");
		}
		else if(strcmp(msg->topic, EMOTION_REQUEST_TOPIC) == 0){
			syslog(LOG_INFO, "Received request to send current emotion");
			char response[100];
			sprintf(response, "{ \"emotion\": \"%s\" }", current_emotion);
			syslog(LOG_INFO, "SENDING EMOTION JSON: %s", response);
			ret = mosquitto_publish(mosq, NULL, EMOTION_RESPONSE_TOPIC, strlen(response), response, 0, false);
			if(ret != MOSQ_ERR_SUCCESS){
				syslog(LOG_ERR, "Error publishing current emotion: %s", mosquitto_strerror(ret));
			}
		}
		else{
			printf("olleh1\n");
			char topic[50];
			strcpy(topic, msg->topic);

			printf("olleh2\n");
			char *topic_start = strtok(topic, MQTT_TOPIC_SEPERATOR);
			printf("olleh3\n");
			char *topic_mid = strtok(NULL, MQTT_TOPIC_SEPERATOR);
			printf("olleh4\n");
			char *topic_end = strtok(NULL, MQTT_TOPIC_SEPERATOR);
			printf("olleh5\n");

			find_and_start_lua_subscribe_script(topic_end, msg->payload);
			printf("olleh6\n");
		}
}
