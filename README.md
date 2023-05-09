# RUTX_prog_for_HASS_comms
Program for communicating with Home Assistant using MQTT.  
The program has the following third-party dependencies: libargp, libmosquitto, libubus, libubox, libblobmsg-json, liblua  
  
### Options:
-a --broker-address=BROADDR (Required. MQTT broker IP address.)  
-p --broker-port=BROPORT (Required. MQTT broker port.)  
-U --username=USR (Optional. MQTT account's username. Might not work without this)  
-P --password=PASS (Optional. MQTT account's password. Might not work without this)  
-b --pub-modules=PUBMODS (Optional. Comma seperated Publish type Lua module names (without the extension). By declaring them they will be called when routinely publishing data.)  
-s --sub-modules=SUBMODS (Optional. Comma seperated Subscribe type Lua module names (without the extension). By declaring them an MQTT topic will be created and listened to.)  
-D --daemon (Optional. Launch program as a background process)  
