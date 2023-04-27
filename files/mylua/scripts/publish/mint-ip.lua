require "ubus"
require "luci.jsonc"

local ubus_conn

function init()
    ubus_conn = ubus.connect()
    if not ubus_conn then
       error("Failed to connect to ubus")
    end
end
function get_data()
    local path, method, object = "uci", "get", {}
    object.config = "network"
    object.section = "lan"
    object.option = "ipaddr"

    local results = ubus_conn:call(path, method, object)
    if not results then
        error("Failed to call ubus with path: "..path..", and method: "..method)
    end

    local output = luci.jsonc.stringify(results)

    return output
end
function destroy()
    ubus_conn:close()
end