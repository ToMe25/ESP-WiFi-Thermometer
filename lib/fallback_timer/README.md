# Fallback Timer
This tiny library contains an ESP8266 NonOS definition of the ESP RTOS `esp_timer_get_time` function.  
This fallback version only works on the ESP8266 NonOS SDK.

**Warning**: This implementation only works if its called more than once every hour.
