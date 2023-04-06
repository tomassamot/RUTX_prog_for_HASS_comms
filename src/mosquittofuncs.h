

#ifndef MOSQUITTOFUNCS_H
#define MOSQUITTOFUNCS_H

#include <mosquitto.h>

int mosq_connect(struct mosquitto **mosq, char *broker, int port, const char *client_id, char *username, char *password);
int mosq_loop(struct mosquitto *mosq, const char *topic, char *payload);
int mosq_disconnect(struct mosquitto *mosq);

#endif