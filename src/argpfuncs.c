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
    {"pub-modules", 'b', "PUBMODS", 0, "Optional. Comma seperated Publish type Lua module names (without the extension). By declaring them they will be called when routinely publishing data." },
    {"sub-modules", 's', "SUBMODS", 0, "Optional. Comma seperated Subscribe type Lua module names (without the extension). By declaring them an MQTT topic will be created and listened to." },
    {"daemon", 'D', 0, 0, "Launch program as a background process"},
    { 0 }
};
static struct argp argp = { options, parse_opt, args_doc, doc };


void start_parser(int argc, char **argv, struct arguments *arguments)
{
    argp_parse(&argp, argc, argv, 0, 0, arguments);
}

static void set_mods(struct ll_module **list, char *arg)
{
	char *without_quotation = strtok(arg, "\"");
	syslog(LOG_INFO, "without_quotation: %s\n", without_quotation);
	char *token = strtok(without_quotation, ",");
	while(token != NULL){
		syslog(LOG_INFO, "token: %s\n", token);
		struct ll_module *module = ll_module_create(token);
		ll_module_add(list, module);
		token = strtok(NULL, ",");
	}
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;
	struct ll_module *list = NULL, *module = NULL;
	int num, is_first = 1;
	char *token;

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

		case 'b':
			set_mods(&list, arg);
			arguments->pub_modules = list;
			break;
		
		case 's':
			set_mods(&list, arg);
			arguments->sub_modules = list;
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