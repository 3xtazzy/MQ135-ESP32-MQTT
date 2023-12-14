# MQ135-ESP32-MQTT
An example of MQ135 sensor on ESP32-DEV with MQTT Integration

MQ135 data transfered with analog pin, that's why you need to calculate the value from the sensor output manually. In this example, we are calculating PPM for measuring air quality index.

Formula can be found in https://github.com/NuclearPhoenixx/MQ135

MQ135 pin connection
- VCC to 3v3
- GND to GND
- A0 to D35
