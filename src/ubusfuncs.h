#include <libubox/blobmsg_json.h>
#include <libubus.h>

enum {
	TOTAL_MEMORY,
	FREE_MEMORY,
	SHARED_MEMORY,
	BUFFERED_MEMORY,
	__MEMORY_MAX,
};

enum {
	MEMORY_DATA,
	__INFO_MAX,
};

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

static const struct blobmsg_policy memory_policy[__MEMORY_MAX] = {
	[TOTAL_MEMORY] = { .name = "total", .type = BLOBMSG_TYPE_INT64 },
	[FREE_MEMORY] = { .name = "free", .type = BLOBMSG_TYPE_INT64 },
	[SHARED_MEMORY] = { .name = "shared", .type = BLOBMSG_TYPE_INT64 },
	[BUFFERED_MEMORY] = { .name = "buffered", .type = BLOBMSG_TYPE_INT64 },
};

static const struct blobmsg_policy info_policy[__INFO_MAX] = {
	[MEMORY_DATA] = { .name = "memory", .type = BLOBMSG_TYPE_TABLE },
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

int connect_to_ubus(struct ubus_context **ctx);
struct blob_attr* get_memory_info(struct ubus_context *ctx);
void disconnect_from_ubus(struct ubus_context *ctx);
int get_static_ip(struct ubus_context *ctx);
int set_new_static_ip(struct ubus_context *ctx, char *ip);
int commit_uci_changes(struct ubus_context *ctx, const char *config);
