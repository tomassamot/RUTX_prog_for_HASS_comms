#include <syslog.h>

#include "ubusfuncs.h"

int ret;

int connect_to_ubus(struct ubus_context **ctx)
{
    *ctx = ubus_connect(NULL);
    if(ctx == NULL)
        return -1;
    return 0;
}
void disconnect_from_ubus(struct ubus_context *ctx)
{
    ubus_free(ctx);
}
int set_new_static_ip(struct ubus_context *ctx, char *ip)
{
	struct blob_buf b = {};
	void *o2;
	uint32_t id;

    ret = 0;

	blob_buf_init(&b, 0);
	
	blobmsg_add_string(&b, "config", "network");
	blobmsg_add_string(&b, "section", "lan");
	o2 = blobmsg_open_table(&b, "values");
	blobmsg_add_string(&b, "ipaddr", ip);
	blobmsg_close_table(&b, o2);

	if (ubus_lookup_id(ctx, "uci", &id)) {
		syslog(LOG_ERR, "Failed to find id of command 'uci'");
        ret=1;
	}
	if (ubus_invoke(ctx, id, "set", b.head, NULL, NULL, 3000)) {
		syslog(LOG_ERR, "Failed to invoke 'set' of 'uci'");
        ret=2;
	}
	blob_buf_free(&b);

	return ret;
}
int commit_uci_changes(struct ubus_context *ctx, const char *config)
{
	struct blob_buf c = {};
	uint32_t id;

    ret = 0;
    
    blob_buf_init(&c, 0);

    if(config != NULL)
	    blobmsg_add_string(&c, "config", "network");

	if (ubus_lookup_id(ctx, "uci", &id)) {
		syslog(LOG_ERR, "Failed to find id of command 'uci'");
        ret=1;
	}
	if (ubus_invoke(ctx, id, "commit", c.head, NULL, NULL, 3000)) {
		syslog(LOG_ERR, "Failed to invoke 'commit' of 'uci'");
		ret=2;
	}

	blob_buf_free(&c);

	return 0;
}