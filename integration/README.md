# ESP-WiFi-Thermometer Integration
This directory contains info on all the services for which this program has builtin support.  
Some of these require some configuration changes on the other side to work.  
This file only contains basic information on the integration, and each integration has its own markdown file explaining how to use it.  
Click the header of an integration to open its markdown file.

## [Grafana](./grafana.md)
This project contains a [grafana](https://grafana.com/) dashboard to visualize the data from the [prometheus](https://prometheus.io/) integration in [grafana](https://grafana.com/).

## [Home-Assistant](./home-assistant.md)
This project contains an explanation with examples for how to use the [MQTT](https://mqtt.org/) integration to show the measurements in [home-assistant](https://www.home-assistant.io/).

## [Kube-Prometheus](./kube-prometheus.md)
This project contains the [kubectl](https://kubernetes.io/docs/reference/kubectl/overview/) manifest and metrics endpoint required to make [kube-prometheus](https://github.com/prometheus-operator/kube-prometheus) automatically collect measurements from this program.

## [MQTT](./mqtt.md)
This project has the capability to publish measurements to a [MQTT](https://mqtt.org/) broker/server.

## [Prometheus](./prometheus.md)
This project contains the metrics endpoint to allow [prometheus](https://prometheus.io/) to scrape the measurements.

## [Prometheus-Pushgateway](./prometheus-pushgateway.md)
This project can push its measurements to a [prometheus-pushgateway](https://github.com/prometheus/pushgateway).
