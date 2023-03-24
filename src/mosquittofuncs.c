#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "mosquittofuncs.h"

static void on_connect(struct mosquitto *mosq, void *obj, int reason_code);
static void on_publish(struct mosquitto *mosq, void *obj, int mid);

int ret = 0;

int mosq_connect(struct mosquitto **mosq, char *broker, int port, char *client_id, char *username, char *password)
{
	mosquitto_lib_init();

	*mosq = mosquitto_new(client_id, true, NULL);
	if(mosq == NULL){
		syslog(LOG_ERR, "Error: Out of memory.");
		return 1;
	}

	mosquitto_connect_callback_set(*mosq, on_connect);
	mosquitto_publish_callback_set(*mosq, on_publish);
    mosquitto_username_pw_set(*mosq, username, password);

	ret = mosquitto_connect(*mosq, broker, port, 60);
    if(ret != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(*mosq);
		syslog(LOG_ERR, "Error: %s", mosquitto_strerror(ret));
		return 1;
	}

	ret = mosquitto_loop_start(*mosq);
	if(ret != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(*mosq);
		syslog(LOG_ERR, "Error: %s", mosquitto_strerror(ret));
		return 1;
	}

	return ret;
}
int mosq_loop(struct mosquitto *mosq, char *topic, char *payload)
{
	ret = mosquitto_publish(mosq, NULL, topic, strlen(payload), payload, 0, false);
	if(ret != MOSQ_ERR_SUCCESS){
		syslog(LOG_ERR, "Error publishing: %s", mosquitto_strerror(ret));
		return ret;
	}
	return 0;
}
int mosq_disconnect(struct mosquitto *mosq)
{
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}

static void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	syslog(LOG_INFO, "Connected successfully");
}


static void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
	syslog(LOG_INFO, "Publishing message to HA");
}