# Overview 

This project is to connect an EPSolar/EPEver Tracer Solar Charge Controller to an MQTT server using an ESP8266 based NodeMCU, and a RS485 modbus adapter. A lot of inspiration was taken from https://github.com/jaminNZx/Tracer-RS485-Modbus-Blynk, which takes the information and pushes to the Blynk service. With MQTT however, I can then incorporate it easily into my OpenHAB home automation, and also graph the readings using Grafana.

# Hardware
I am developing this on a NodeMCU v1, but see no reason why it won't work with other similar hardware.



