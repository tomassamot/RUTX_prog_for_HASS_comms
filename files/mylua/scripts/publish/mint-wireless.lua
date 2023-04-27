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
    local path, method, object = "network.wireless", "status", {}

    local results = ubus_conn:call(path, method, object)

    if not results then
        error("Failed to call ubus with path: "..path..", and method: "..method)
    end

    for k1,radio in pairs(results) do
        for k2, interface in ipairs(radio.interfaces) do
            if interface.ifname == nil then
                interface.conn_devices = ""
            else
                interface.conn_devices = get_connected_devices(interface.ifname)
                interface.banned_devices = get_banned_devices(interface.ifname)
            end
        end
    end

    local output = luci.jsonc.stringify(results)

    return output
end
function destroy()
    ubus_conn:close()
end

function get_connected_devices(ifname)
    local cmd = "iwinfo "..ifname.." assoclist"
    local handle = io.popen(cmd)
    local result = handle:read("*a")
    handle:close()
    local conn_devices = ""
    local pattern = [[[a-zA-Z0-9]+:[a-zA-Z0-9]+:[a-zA-Z0-9]+:[a-zA-Z0-9]+:[a-zA-Z0-9]+:[a-zA-Z0-9]+]]
    for str in string.gmatch(result, pattern) do
        if conn_devices == "" then
            conn_devices = str
        else
            conn_devices = conn_devices.."; "..str
        end
    end
    return conn_devices
end

function get_banned_devices(ifname)
    local path, method, object = "hostapd."..ifname, "list_bans", {}
    local results = ubus_conn:call(path, method, object)

    if not results then
        error("Failed to call ubus with path: "..path..", method: "..method..", and object: "..object)
    end

    local banned_devices = ""
    for k, str in ipairs(results.clients) do
        if banned_devices == "" then
            banned_devices = str
        else
            banned_devices = banned_devices.."; "..str
        end
    end

    return banned_devices
end