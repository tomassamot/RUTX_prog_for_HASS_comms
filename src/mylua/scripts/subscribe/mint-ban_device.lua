
require "ubus"
-- require "uloop"

package.path = package.path .. ";/usr/mylua/scripts/packages/json.lua"
json = require "json"

local ubus_conn

function init()
    ubus_conn = ubus.connect()
    if not ubus_conn then
       error("Failed to connect to ubus")
    end
end

function set_data(json_str)
    print("json_str: "..json_str)

    -- data = json.decode(json_str)
    local data = json.decode(json_str)

    print("data.ban_time: "..data.ban_time)

    local path, method, object = "hostapd."..data.ifname, "del_client", {}
    object.addr = data.addr
    object.reason = 1
    object.deauth = false
    object.ban_time = data.ban_time

    print("path: "..path..", method: "..method..", object.ban_time: "..object.ban_time)

    ubus_conn:call(path, method, object)
end
function destroy()
    ubus_conn:close()
end

function script_path()
    local str = debug.getinfo(2, "S").source:sub(2)
    return str:match("(.*/)")
end