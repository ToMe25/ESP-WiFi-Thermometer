# Prometheus Pushgateway integration
This project is designed to be able to push its measurements as well as web page usage stats to a [prometheus pushgateway](https://github.com/prometheus/pushgateway) server.  
This can be preferable over [prometheus](https://prometheus.io/) scraping metrics from the metrics endpoint in two cases.
 1. When your ESP often changes its IP address or you are often using a different ESP.
 2. When you are using [deep sleep mode](../README.md#deep-sleep-mode) and thus the web server isn't available.

## Setup
These instructions assume you already have a running [prometheus](https://prometheus.io/) instance as well as a [prometheus pushgateway](https://github.com/prometheus/pushgateway) server that is integrated with it.
 1. Make sure `ENABLE_PROMETHEUS_PUSH` is set to 1 in config.h
 2. Set `PROMETHEUS_PUSH_ADDR` in config.h to the address of your [prometheus pushgateway](https://github.com/prometheus/pushgateway) instance.  
    Also adjust `PROMETHEUS_PUSH_PORT` if your [prometheus pushgateway](https://github.com/prometheus/pushgateway) instance is using a non standard port.(default is port 9091)
 3. Set `PROMETHEUS_PUSH_JOB`, `PROMETHEUS_PUSH_INSTANCE`, and `PROMETHEUS_PUSH_NAMESPACE` if you want your metrics to be pushed to a different job or namespace.  
    Leaving them as is should be fine for most usecases.
 4. If you want to change how often measurements are pushed change `PROMETHEUS_PUSH_INTERVAL` in config.h.(default is 30 seconds)
 5. Build this project and flash it to your ESP.

After following these instructions you should get data after about 30 seconds.
