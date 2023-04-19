
require "ubus"

package.path = package.path .. ";/usr/mylua/scripts/packages/json.lua"
json = require "json"

local ubus_conn

function init()
    ubus_conn = ubus.connect()
    if not ubus_conn then
       error("Failed to connect to ubus")
    end
end
function get_data()
    local path, method = "system", "info"
    local results = ubus_conn:call(path, method, {})
    if not results then
        error("Failed to call ubus with path: "..path..", and method: "..method)
    end

    local output = json.encode(results.memory)
    return output
end
function destroy()
    ubus_conn:close()
end

function script_path()
    local str = debug.getinfo(2, "S").source:sub(2)
    return str:match("(.*/)")
end