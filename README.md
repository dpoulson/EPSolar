# Overview 

This project is to connect an EPSolar/EPEver Tracer Solar Charge Controller to an MQTT server using an ESP8266 based Wemos D1 Mini, and a RS485 modbus adapter. A lot of inspiration was taken from https://github.com/jaminNZx/Tracer-RS485-Modbus-Blynk, which takes the information and pushes to the Blynk service. With MQTT however, I can then incorporate it easily into my OpenHAB home automation, and also graph the readings using Grafana.

# Hardware
I am developing this on Wemos D1 Mini, but see no reason why it won't work with other similar hardware.

# MQTT
It will talk back to any MQTT server using the Homie standard, so auto detected in things like OpenHab

# Usage

Copy the config.h-default to config.h and edit the parameters. If you have more than one solar module, then you can adjust the ID number in each client.publish line. Plans are to at some point make this a single variable, but that is in testing.

# Incorporating into OpenHAB

As it uses the Homie standard, it will automatically be detected by OpenHab

# Author

Darren Poulson \<darren.poulson@gmail.com\>

Further reading: http://60chequersavenue.net/wordpress/2017/07/pretty-solar-graphs/


