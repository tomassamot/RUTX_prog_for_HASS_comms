#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <argp.h>

#include "argpfuncs.h"


static error_t parse_opt (int key, char *arg, struct argp_state *state);

const char *argp_program_version =
	"1.0";
const char *argp_program_bug_address =
    "/dev/null";

static char doc[] =
    "A program for my Home Assistant integration";

static char args_doc[] = "";

static struct argp_option options[] = {
    {"broker-address", 'a', "BROADDR", 0, "Required. MQTT broker IP address" },
    {"broker-port", 'p', "BROPORT", 0, "Required. MQTT broker port" },
    {"username", 'U', "USR", 0, "Optional. MQTT account's username. Might not work without this" },
    {"password", 'P', "PASS", 0, "Optional. MQTT account's password. Might not work without this" },
    {"daemon", 'D', 0, 0, "Launch program as a background process"},
    { 0 }
};
static struct argp argp = { options, parse_opt, args_doc, doc };


void start_parser(int argc, char **argv, struct arguments *arguments)
{
    argp_parse(&argp, argc, argv, 0, 0, arguments);
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;
	int num;

  	switch (key){
		case 'a':
			strcpy(arguments->broker_address, arg);
			break;

		case 'p':
			sscanf(arg, "%d", &num);
			arguments->broker_port = num;
			break;

		case 'U':
			strcpy(arguments->username, arg);
			break;

		case 'P':
			strcpy(arguments->password, arg);
			break;

		case 'D':
			arguments->daemon = 1;
			break;

		case ARGP_KEY_ARG:
			argp_usage(state);
			break;

		case ARGP_KEY_END:
			if(strcmp(arguments->broker_address, "") == 0 || arguments->broker_port == 0)
				argp_usage(state);
			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}
  	return 0;
}