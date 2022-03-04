# TODO
A list of todos to be implemented at a later date.  
Not all of these are necessarily going to be implemented at all.

## General
 * Change main to an actual class
 * Keep some history of measurements(required for some other todos)
 * Fix ESP8266 ipv6 support
 * Fix ESP8266 WiFi scan support
 * Merge wifissid.txt and wifipass.txt into wificreds.txt and merge mqttuser.txt and mqttpass.txt into mqttcreds.txt

## Web Interface
 * Add error message to log and web interface if measurement fails
 * Add a javescript temperature and humidity graph to the web interface?
 * Remove not measured values(for example humidity for the DS18B20)
 * Add degrees fahrenheit mode to web interface
 * Allow caching of static resources using a hash of their content as an ETag

## Integration
 * Find a way to make the kubernetes service(for prometheus) automatically point to the esp(mDNS?)
 * Add missed measurements to the next prometheus scrape or push with a timestamp
 * Add optional MQTT broker if dsm is disabled
 * Add MQTT discovery support(https://www.home-assistant.io/docs/mqtt/discovery/)
