# TODO
A list of todos to be implemented at a later date.  
Not all of these are necessarily going to be implemented at all.

## General
 * Change main to an actual class
 * Keep some history of measurements(required for some other todos)

## Web Interface
 * Add error message to log and web interface if measurement fails
 * Add a javescript temperature and humidity graph to the web interface?

## Integration
 * Find a way to make the kubernetes service(for prometheus) automatically point to the esp(mDNS?)
 * Add prometheus push integration(Would be a workaround for continuously changing ESP ips)
 * Add missed measurements to the next prometheus scrape or push with a timestamp
 * Add [Home Assistant](https://www.home-assistant.io/) integration

## Hardware
 * Add ESP8266 support(I don't have one yet, so not any time soon)
