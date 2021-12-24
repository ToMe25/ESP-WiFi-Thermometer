# Description
This project measures the temperature and relative humidity using a DHT22 and shows it on a simple web interface.  
The web interface updates itself every 2 seconds(the max refresh rate possible with a single DHT22) using a small javascript.  
This is done by querying /data.json on the esp and replacing the shown values using that.
A screenshot of the web interface:  
![web interface](https://raw.githubusercontent.com/ToMe25/ESP32-DHT22/master/images/web_interface.png)

# Requirements
 1. ESP32-devkitc
 2. DHT22
 3. 10KΩ Resistor
 4. USB-Micro-B tp USB-A cable
 5. PC with a working [platformio](https://platformio.org/) installation

# Getting Started
These are the steps required to use this project and flash it onto an ESP32.
 * Attach the DHT22 to the ESP32 like this:

|ESP32 Pin     |DHT22 Pin|
|--------------|---------|
|5V            |VCC      |
|5V(10KΩ),GPIO5|Data     |
|GND           |GND      |

 * Attach the ESP32 to your PC using a USB-Micro-B tp USB-A cable.
 * Create a wifissid.txt file containing the SSID of the wifi to connect to.
 * Create a wifipass.txt file containing the Passphrase for the wifi to connect to.
 * Create a otapass.txt file containing the password for [ArduinoOta](https://www.arduino.cc/reference/en/libraries/arduinoota/), allowing your to upload modified versions over wifi.
 * Make sure these three files don't end with an empty line.
 * Build this project and flash it to the ESP32 by running `pio run -t upload -e esp32dev` to upload over the cable connection.
 * To upload over WiFi after this project is installed run `pio run -t upload -e ota`.
 * Run `pio device monitor` to open a serial connection.
 * Wait a few seconds for the ESP32 to finish booting.
 * Enter `ip` and press return to get the device ip.
 * Enter the IPv4 ip(probably 192.168.xxx.xxx) returned by this into your browser to see the web interface, showing your current temperature and relative humidity.

# Kube-Prometheus integration
This repository contains the necessary kubectl manifests and grafana dashboard to be used with [kube-prometheus](https://github.com/prometheus-operator/kube-prometheus).  
The necessary metrics for this are provided on /metrics on port 80 on the esp by default.  
To install this prometheus integration simply run `kubectl apply -f integration/kube-prometheus-scrape.yaml` in the project root on the server.  
This configures kube-prometheus to scrape /metrics on the esp(the IP has to be set in the yaml) every 10 seconds.  
This change might take a few minutes until it applies, so don't worry if you don't have data immediately.  
After this, to install the grafana dashboard, follow these steps:
 * Open your grafana instance and Log in as a user with the required permissions to create a dashboard.
 * Hover over the '+' on the left side of the page.
 * Click "Import".
 * Click "Upload .json file".
 * Select the "grafana-dashboard.json" file in the "kube-prometheus" folder.
 * Click "Import".

A screenshot of the grafana dasboard:  
![dashboard](https://raw.githubusercontent.com/ToMe25/ESP32-DHT22/master/images/grafana_dashboard.png)
