# RefrigeratorControl
Refrigerator Control using Sonnoff Dual (nodemcu)

Basic requirements:

   req-001: power on the refrigerator if temp read > temp set + deltapos;
   req-002: power off the refrigerator if temp read < temp set - deltaneg;
   req-003: the command to power on/off the refrigerator is at 1 second;
   req-004: read the DS18B20 every 1 second;
   req-005: read the temp and set the temperature through the html site
   req-007: update the software through the OTA;
   req-008.1: mqtt: send the temp read, set temperature and relay status
   req-008.2:  mqtt:  set the temperatures through the mqtt, fix the node-red to retain the publish
   req-010: receive set temperature through mqtt and save in the memory - ready but mqtt has priority, once that it returns, it will use the mqtt stored, better to disable the mqtt only save in the html
   req-009.1: figure out how to record negative set condition, as -10, the set goes from -30 to 30
   req-012: add a delay power up of the refrigerator during power on, > 30 seconds;
   req-014.0: to monitor more than one temperature sensor and select where to use, it was used eppron, it was select according the Number of the sensor, it should be according the address
