

#ifndef MOSQUITTOFUNCS_H
#define MOSQUITTOFUNCS_H

#include <mosquitto.h>
#include "ubusfuncs.h"

int mosq_connect(struct mosquitto **mosq, struct ubus_context *ubus_ctx, char *broker, int port, const char *client_id, char *username, char *password);
int mosq_loop(struct mosquitto *mosq, const char *topic, char *payload);
int mosq_disconnect(struct mosquitto *mosq);

#endif