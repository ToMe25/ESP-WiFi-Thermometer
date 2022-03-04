# Prometheus integration
This project is designed to be easy to integrate with [prometheus](https://prometheus.io/).  
For this it contains a metrics endpoint(/metrics on the web server) which contains the measurements as well as usage statistics for the web pages.  
Alternatively there is also the [prometheus pushgateway integration](./prometheus-pushgateway.md) which makes this program push these metrics to a [prometheus pushgateway](https://github.com/prometheus/pushgateway) instance to allow use in [prometheus](https://prometheus.io/), but more about that in [its own file](./prometheus-pushgateway.md).

## Setup
This assumes you already have a running [prometheus](https://prometheus.io/) server.
 1. Make sure `ENABLE_PROMETHEUS_SCRAPE_SUPPORT` is set to 1 in config.h.
 2. Build and flash this program again if it wasn't enabled before.
 3. Add something like the example below to your `prometheus.yml` configuration file in the `scrape_configs` section.
 4. Reload your prometheus config, for example by restarting prometheus.

**Example prometheus.yml config section:**

```yml
  - job_name: esp-wifi-thermometer
    static_configs:
    - targets: ['ESP_IP:80']
```

Make sure to replace `ESP_IP` with the ip address of your ESP.  
This program writes the local IP address of the ESP to the Serial console on startup.

If you are using a [prometheus](https://prometheus.io/) on [kubernetes](https://kubernetes.io/) (for example [kube-prometheus](https://github.com/prometheus-operator/kube-prometheus) or [cluster-monitoring](https://github.com/carlosedp/cluster-monitoring)) you can use the [kube prometheus integration](./kube-prometheus.md) instead.
