require "ubus"
require "luci.jsonc"

local ubus_conn

function init()
    ubus_conn = ubus.connect()
    if not ubus_conn then
       error("Failed to connect to ubus")
    end
end

function set_data(json_str)
    local data = luci.jsonc.parse(json_str)

    local path, method, object = "uci", "set", {}
    object.config = "network"
    object.section = "lan"
    object.values = {}
    object.values.ipaddr = data["new_ip"]
    ubus_conn:call(path, method, object)

    path, method, object = "uci", "commit", {}
    object.config = "network"
    ubus_conn:call(path, method, object)
end
function destroy()
    ubus_conn:close()
end