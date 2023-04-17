
require "ubus"
-- require "uloop"

package.path = package.path .. ";/usr/mylua/scripts/json.lua"
json = require "json"

local ubus_conn
local program_is_killed = false

function init()
    ubus_conn = ubus.connect()
    if not ubus_conn then
       error("Failed to connect to ubus")
    end
end
function get_data()
    local path, method, object = "network.wireless", "status", {}

    local results = ubus_conn:call(path, method, object)

    if not results then
        error("Failed to call ubus with path: "..path..", and method: "..method)
    end

    for k1,radio in pairs(results) do
        for k2, interface in ipairs(radio.interfaces) do
            local cmd = "iwinfo "..interface.ifname.." assoclist"
            local handle = io.popen(cmd)
            local result = handle:read("*a")
            handle:close()
            local conn_devices = ""
            local pattern = [[[a-zA-Z0-9]+:[a-zA-Z0-9]+:[a-zA-Z0-9]+:[a-zA-Z0-9]+:[a-zA-Z0-9]+:[a-zA-Z0-9]+]]
            for str in string.gmatch(result, pattern) do
                conn_devices = conn_devices.." "..str
            end
            interface.conn_devices = conn_devices
        end
    end

    local output = json.encode(results)

    return output
end
function destroy()
    ubus_conn:close()
end

function script_path()
    local str = debug.getinfo(2, "S").source:sub(2)
    return str:match("(.*/)")
end