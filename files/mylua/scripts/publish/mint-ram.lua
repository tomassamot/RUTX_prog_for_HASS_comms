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
    local path, method, object = "system", "info", {}
    
    local results = ubus_conn:call(path, method, object)
    if not results then
        error("Failed to call ubus with path: "..path..", method: "..method..", and object: "..object)
    end

    local output = luci.jsonc.stringify(results.memory)
    return output
end
function destroy()
    ubus_conn:close()
end