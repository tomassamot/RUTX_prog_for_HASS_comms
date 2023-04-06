#include <libubox/blobmsg_json.h>
#include <libubus.h>





/*
*
* Policies for getting RAM information
*
*/

enum {
	TOTAL_MEMORY,
	FREE_MEMORY,
	SHARED_MEMORY,
	BUFFERED_MEMORY,
	AVAILABLE_MEMORY,
	CACHED_MEMORY,
	__MEMORY_MAX,
};

enum {
	MEMORY_DATA,
	__INFO_MAX,
};

static const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
	[TOTAL_MEMORY] = { .name = "total", .type = BLOBMSG_TYPE_INT64 },
	[FREE_MEMORY] = { .name = "free", .type = BLOBMSG_TYPE_INT64 },
	[SHARED_MEMORY] = { .name = "shared", .type = BLOBMSG_TYPE_INT64 },
	[BUFFERED_MEMORY] = { .name = "buffered", .type = BLOBMSG_TYPE_INT64 },
	[AVAILABLE_MEMORY] = { .name = "available", .type = BLOBMSG_TYPE_INT64 },
	[CACHED_MEMORY] = { .name = "cached", .type = BLOBMSG_TYPE_INT64 },
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
	[MEMORY_DATA] = { .name = "memory", .type = BLOBMSG_TYPE_TABLE },
};

/*
*
* Policies for setting a new IP address
*
*/

enum{
	VALUE,
	__IPADDR_MAX
};

enum{
	GET_CONFIG,
	GET_SECTION,
	GET_OPTION,
	__UCI_GET_MAX,
};

enum{
	SET_CONFIG,
	SET_SECTION,
	SET_OPTION,
	SET_VALUES,
	__UCI_SET_MAX,
};

static const struct blobmsg_policy ipaddr_policy[__IPADDR_MAX] = {
	[VALUE] = { .name = "value", .type = BLOBMSG_TYPE_STRING }
};

static const struct blobmsg_policy get_ipaddr_policy[__UCI_GET_MAX] = {
	[GET_CONFIG] = { .name = "config", .type =  BLOBMSG_TYPE_STRING },
	[GET_SECTION] = { .name = "section", .type =  BLOBMSG_TYPE_STRING },
	[GET_OPTION] = { .name = "option", .type =  BLOBMSG_TYPE_STRING },
};
static const struct blobmsg_policy set_ipaddr_policy[__UCI_SET_MAX] = {
	[SET_CONFIG] = { .name = "config", .type =  BLOBMSG_TYPE_STRING },
	[SET_SECTION] = { .name = "section", .type =  BLOBMSG_TYPE_STRING },
	[SET_OPTION] = { .name = "option", .type =  BLOBMSG_TYPE_STRING },
	[SET_VALUES] = { .name = "values", .type =  BLOBMSG_TYPE_TABLE },
};

/*
*
* Policies for getting wireless information
*
*/




static const struct blobmsg_policy status_policy[2] = {
	[0] = { .name = "radio0", .type = BLOBMSG_TYPE_TABLE },
	[1] = { .name = "radio1", .type = BLOBMSG_TYPE_TABLE },
};

static const struct blobmsg_policy radio_policy[4] = {
	[0] = { .name = "up", .type = BLOBMSG_TYPE_BOOL },
	[1] = { .name = "disabled", .type = BLOBMSG_TYPE_BOOL },
	[2] = { .name = "config", .type = BLOBMSG_TYPE_TABLE },
	[3] = { .name = "interfaces", .type = BLOBMSG_TYPE_ARRAY },
};
static const struct blobmsg_policy config_policy[3] = {
	[0] = { .name = "channel", .type = BLOBMSG_TYPE_STRING },
	[1] = { .name = "hwmode", .type = BLOBMSG_TYPE_STRING },
	[2] = { .name = "htmode", .type = BLOBMSG_TYPE_STRING },
};
static const struct blobmsg_policy interface_policy[2] = {
	[0] = { .name = "ifname", .type = BLOBMSG_TYPE_STRING },
	[1] = { .name = "config", .type = BLOBMSG_TYPE_TABLE },
};
static const struct blobmsg_policy ifconfig_policy[2] = {
	[0] = { .name = "ssid", .type = BLOBMSG_TYPE_STRING },
	[1] = { .name = "wifi_id", .type = BLOBMSG_TYPE_STRING },
};


int connect_to_ubus(struct ubus_context **ctx);
struct blob_attr* get_memory_info(struct ubus_context *ctx);
void disconnect_from_ubus(struct ubus_context *ctx);
int get_static_ip(struct ubus_context *ctx);
int set_new_static_ip(struct ubus_context *ctx, char *ip);
int commit_uci_changes(struct ubus_context *ctx, const char *config);
char* get_radio_info(struct blob_attr *status[], int i);