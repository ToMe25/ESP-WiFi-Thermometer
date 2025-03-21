# TODO
A list of todos to be implemented at a later date.  
Not all of these are necessarily going to be implemented at all.

## General
 * Change main to an actual class?
 * Keep some history of measurements(required for some other todos)
 * Fix ESP8266 ipv6 support
 * Fix ESP8266 WiFi scan support
 * Merge wifissid.txt and wifipass.txt into wificreds.txt and merge mqttuser.txt and mqttpass.txt into mqttcreds.txt
 * Improve log messages for when measurements fail
 * Use an asynchronous DHT library(for example https://github.com/bertmelis/esp32DHT/)?
 * Add (stream?) compression to uzlib_gzip_wrapper
 * Change callback based uzlib_ungzip_wrapper to use C++ function objects instead of C function pointers
 * Consider using PIO middleware or SCons compilation callback to generate compressed web files(into build dir?)

## Web Interface
 * Add error message to web interface if measurement fails
 * Add a javescript temperature and humidity graph to the web interface?
 * Remove not measured values(for example humidity for the DS18B20)
 * Add degrees fahrenheit mode to web interface(clientside setting)
 * Add theme switcher to web interface(clientside setting)
 * Web server IPv6 support
 * Add current commit HTTP Server header

## Integration
 * Find a way to make the kubernetes service(for prometheus) automatically point to the esp(mDNS?)
 * Add missed measurements to the next prometheus scrape or push with a timestamp?
 * Add measurement timestamp to measurements for prometheus
 * Add optional MQTT broker if dsm is disabled
 * Add MQTT state json
 * Cleanup MQTT code
 * Add MQTT discovery support(https://www.home-assistant.io/docs/mqtt/discovery/)
 * Dynamically gzip compress metrics page?
 * Implement actual prometheus library as external project
 * Add prometheus info metrics esptherm_network_info, esptherm_module_info, and esptherm_sensor_info
 * Add MQTT metrics to prometheus
 * Add prometheus metrics for HTTP response times and sizes
 * Add prometheus metrics for push statistics, if DSM is disabled
 * Add measurement error metrics
