#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mosquittofuncs.h"

static void on_connect(struct mosquitto *mosq, void *obj, int reason_code);
static void on_publish(struct mosquitto *mosq, void *obj, int mid);

int ret = 0;

int mosq_connect(struct mosquitto **mosq, char *broker, int port, char *client_id, char *username, char *password)
{
    /* Required before calling other mosquitto functions */
	mosquitto_lib_init();

	/* Create a new client instance.
	 * id = NULL -> ask the broker to generate a client id for us
	 * clean session = true -> the broker should remove old sessions when we connect
	 * obj = NULL -> we aren't passing any of our private data for callbacks
	 */
	*mosq = mosquitto_new(client_id, true, NULL);
	if(mosq == NULL){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}

	/* Configure callbacks. This should be done before connecting ideally. */
	mosquitto_connect_callback_set(*mosq, on_connect);
	mosquitto_publish_callback_set(*mosq, on_publish);
    mosquitto_username_pw_set(*mosq, username, password);

	/* Connect to test.mosquitto.org on port 1883, with a keepalive of 60 seconds.
	 * This call makes the socket connection only, it does not complete the MQTT
	 * CONNECT/CONNACK flow, you should use mosquitto_loop_start() or
	 * mosquitto_loop_forever() for processing net traffic. */
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
int mosq_loop(struct mosquitto *mosq, char *topic, char *payload)
{
    /* At this point the client is connected to the network socket, but may not
	 * have completed CONNECT/CONNACK.
	 * It is fairly safe to start queuing messages at this point, but if you
	 * want to be really sure you should wait until after a successful call to
	 * the connect callback.
	 * In this case we know it is 1 second before we start publishing.
	 */

	/* Publish the message
	 * mosq - our client instance
	 * *mid = NULL - we don't want to know what the message id for this message is
	 * topic = "example/temperature" - the topic on which this message will be published
	 * payloadlen = strlen(payload) - the length of our payload in bytes
	 * payload - the actual payload
	 * qos = 2 - publish with QoS 2 for this example
	 * retain = false - do not use the retained message feature for this message
	 */
	ret = mosquitto_publish(mosq, NULL, topic, strlen(payload), payload, 2, false);
	if(ret != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(ret));
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

/* Callback called when the client receives a CONNACK message from the broker. */
static void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	/* Print out the connection result. mosquitto_connack_string() produces an
	 * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	 * clients is mosquitto_reason_string().
	 */
	printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
	if(reason_code != 0){
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */
		mosquitto_disconnect(mosq);
	}

	/* You may wish to set a flag here to indicate to your application that the
	 * client is now connected. */
}


/* Callback called when the client knows to the best of its abilities that a
 * PUBLISH has been successfully sent. For QoS 0 this means the message has
 * been completely written to the operating system. For QoS 1 this means we
 * have received a PUBACK from the broker. For QoS 2 this means we have
 * received a PUBCOMP from the broker. */
static void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
	printf("Message with mid %d has been published.\n", mid);
}