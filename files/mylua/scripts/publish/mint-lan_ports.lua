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
    local path, method, object = "port_events", "show", {}

    local results = ubus_conn:call(path, method, object)
    if not results then
        error("Failed to call ubus with path: "..path..", method: "..method..", and object: "..object)
    end

    local lans = {}
    for k, val in pairs(results) do
        if string.match(k, "LAN") then
            lans[k] = val
        end
    end

    local output = luci.jsonc.stringify(lans)

    return output
end
function destroy()
    ubus_conn:close()
end