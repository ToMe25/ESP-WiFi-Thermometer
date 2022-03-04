# MQTT integration
This program is able to automatically publish its measurements to a [MQTT](https://mqtt.org/) broker/server.  
This is done on a fixed interval.  
This project does not currently support encrypted [MQTT](https://mqtt.org/), but it does support [MQTT](https://mqtt.org/) authentication.

## Setup
These setup instructions require an already setup [MQTT](https://mqtt.org/) broker/server(for example [mosquitto](https://mosquitto.org/)).
 1. Make sure `ENABLE_MQTT_PUBLISH` is set to 1 in config.h.
 2. Write your [MQTT](https://mqtt.org/) username to mqttuser.txt and your [MQTT](https://mqtt.org/) password to mqttpass.txt.
 3. Set `MQTT_BROKER_ADDR` in config.h to the address of your [MQTT](https://mqtt.org/) broker/server.  
    Also set `MQTT_BROKER_PORT` to the port your [MQTT](https://mqtt.org/) broker/server if you are using a non standard port.(The default port is 1883)
 4. Set `MQTT_PUBLISH_ANONYMOUS` in config.h to 1 if you wish to disable [MQTT](https://mqtt.org/) authentication.
 5. Set `MQTT_TOPIC_NAMESPACE` in config.h if you wish to use something other then your ESP hostname as the first part of the [MQTT](https://mqtt.org/) topic.
 6. If you want to change how often the measurements are published change `MQTT_PUBLISH_INTERVAL` in config.h.(default interval is 15 seconds)
 7. Build this project and flash it to your ESP.
