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
    object.config = "dhcp"
    object.section = "@dnsmasq[0]"
    object.option = "leasefile"

    local leasefile_results = ubus_conn:call(path, method, object)
    if not leasefile_results then
        error("Failed to call ubus with path: "..path..", method: "..method..", and object: "..object)
    end

    local leases = get_leases(leasefile_results)
    
    local output = luci.jsonc.stringify(leases)

    return output
end
function destroy()
    ubus_conn:close()
end

function get_leases(leasefile_results)
    local cmd = "cat "..leasefile_results.value
    local handle = io.popen(cmd)
    local leasefile_contents = handle:read("*a")
    handle:close()

    local leases = {}
    local i = 1
        for line in string.gmatch(leasefile_contents, "([^\n]+)") do
        table.insert( leases, {})
        local j = 1
        for str in string.gmatch(line, "([^%s]+)") do
            if j == 1 then
            elseif j == 2 then
                leases[i].mac = str
            elseif j == 3 then
                leases[i].ipaddr = str
            elseif j == 4 then
                leases[i].hostname = str
            elseif j == 5 then
            else
                error("Unexpeceted amount of tokens found in leasefile_contents line")
            end
            j=j+1
        end
        i=i+1
    end
    return leases
end