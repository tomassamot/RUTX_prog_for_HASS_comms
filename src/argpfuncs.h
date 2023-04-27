#ifndef ARGPFUNCS_H
#define ARGPFUNCS_H

#include "ll_module.h"

struct arguments{
	char broker_address[20];
	int broker_port;
	char username[50];
	char password[50];
	int daemon;
	struct ll_module *pub_modules;
	struct ll_module *sub_modules;
};



void start_parser(int argc, char **argv, struct arguments *arguments);

#endif
