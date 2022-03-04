# Kube Prometheus integration
This project contains the [prometheus](https://prometheus.io/) metrics endpoint and the [kubectl](https://kubernetes.io/docs/reference/kubectl/overview/) manifest to make [kube-prometheus](https://github.com/prometheus-operator/kube-prometheus) automatically collect the measurements from this project.  
This integration also gives [prometheus](https://prometheus.io/) access to web inteface usage statistics.  
This integration also works for [kube-prometheus](https://github.com/prometheus-operator/kube-prometheus) based projects like [cluster-monitoring](https://github.com/carlosedp/cluster-monitoring).  
This integration should **in theory** also work with every other [kubernetes](https://kubernetes.io/) [prometheus](https://prometheus.io/) integration, however this is untested.

## Setup
This assumes that you already have a running [kube-prometheus](https://github.com/prometheus-operator/kube-prometheus) instance.
 1. Make sure `ENABLE_PROMETHEUS_SCRAPE_SUPPORT` is set to 1 in config.h.
 2. Build and flash this program again if it wasn't enabled before.
 3. Download [kube-prometheus-scrape.yaml](./kube-prometheus-scrape.yaml).(For example running `wget https://raw.githubusercontent.com/ToMe25/ESP-WiFi-Thermometer/master/integration/kube-prometheus-scrape.yaml`.)
 4. Replace `192.168.2.101` in kube-prometheus-scrape.yaml with the ip address of your ESP.
 5. Apply the [kubectl](https://kubernetes.io/docs/reference/kubectl/overview/) manifest by running `kubectl apply -f kube-prometheus-scrape.yaml`.

This example manifest scrapes the metrics every 10 seconds.  
Note: it can take a few minutes until you get measurements after following these steps.
