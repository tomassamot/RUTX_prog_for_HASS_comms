#ifndef ARGPFUNCS_H
#define ARGPFUNCS_H

struct arguments{
	char broker_address[20];
	int broker_port;
	char username[50];
	char password[50];
	int daemon;
};

void start_parser(int argc, char **argv, struct arguments *arguments);

#endif
