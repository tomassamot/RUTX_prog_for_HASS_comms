#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "mosquittofuncs.h"

static void on_connect(struct mosquitto *mosq, void *obj, int reason_code);
static void on_publish(struct mosquitto *mosq, void *obj, int mid);
static void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);
static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);

const char *NEW_IP_TOPIC = "home-assistant/mint/new_ip";
const char *INIT_TOPIC = "home-assistant/mint/init";
const char *EMOTION_REQUEST_TOPIC = "home-assistant/mint/emotion_req";
const char *EMOTION_RESPONSE_TOPIC = "home-assistant/mint/emotion_res";

int ret = 0;
char *current_emotion = "das";

int mosq_connect(struct mosquitto **mosq, char *broker, int port, const char *client_id, char *username, char *password)
{
	*mosq = mosquitto_new(NULL, true, NULL);
	if(*mosq == NULL){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	mosquitto_connect_callback_set(*mosq, on_connect);
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

	/* Run the network loop in a background thread, this call returns quickly. */
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
	ret = mosquitto_publish(mosq, NULL, topic, strlen(payload), payload, 0, false);
	if(ret != MOSQ_ERR_SUCCESS){
		syslog(LOG_ERR, "Error publishing: %s", mosquitto_strerror(ret));
		return ret;
	}
	return 0;
}
int mosq_disconnect(struct mosquitto *mosq)
{
	mosquitto_disconnect(mosq);
	mosquitto_loop_stop(mosq, false);
	mosquitto_destroy(mosq);
	return 0;
}

static void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	if(reason_code != 0){
		// syslog(LOG_ERR, "Failed to connect. Return code: %d", reason_code);
		syslog(LOG_ERR, "Failed to connect. %s", mosquitto_strerror(reason_code));
		mosquitto_disconnect(mosq);
		return;
	}
	syslog(LOG_INFO, "Trying to subscribe...");
	ret = mosquitto_subscribe(mosq, NULL, NEW_IP_TOPIC, 1);
	if(ret != MOSQ_ERR_SUCCESS)
		syslog(LOG_ERR, "Error subscribing: %s", mosquitto_strerror(ret));
	ret = mosquitto_subscribe(mosq, NULL, EMOTION_REQUEST_TOPIC, 1);
	if(ret != MOSQ_ERR_SUCCESS)
		syslog(LOG_ERR, "Error subscribing: %s", mosquitto_strerror(ret));
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
	syslog(LOG_INFO, "%s %d %s\n", msg->topic, msg->qos, (char *)msg->payload);
		if(strcmp(msg->topic, NEW_IP_TOPIC) == 0){
			syslog(LOG_INFO, "Received request to change IP");
			// <...> change router's static IP
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
			syslog(LOG_WARNING, "Received message to a topic, which has no implementation");
		}
}
