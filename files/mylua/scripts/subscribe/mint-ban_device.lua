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

    local path, method, object = "hostapd."..data.ifname, "del_client", {}
    object.addr = data.addr
    object.reason = 1
    object.deauth = false
    object.ban_time = data.ban_time

    ubus_conn:call(path, method, object)
end
function destroy()
    ubus_conn:close()
end