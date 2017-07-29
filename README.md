# Overview 

This project is to connect an EPSolar/EPEver Tracer Solar Charge Controller to an MQTT server using an ESP8266 based NodeMCU, and a RS485 modbus adapter. A lot of inspiration was taken from https://github.com/jaminNZx/Tracer-RS485-Modbus-Blynk, which takes the information and pushes to the Blynk service. With MQTT however, I can then incorporate it easily into my OpenHAB home automation, and also graph the readings using Grafana.

# Hardware
I am developing this on a NodeMCU v1, but see no reason why it won't work with other similar hardware.

# MQTT
By default, this will write to queues under:

/EPSolar/<device ID>/<stat>

# Incorporating into OpenHAB

You will need the MQTT binding installed and configured to connect to the same MQTT instance. Once that is done, you can create an items file similar to this:

     Number EPSolar_Temp "Temperature [%.2f °C]" { mqtt="<[mymosquitto:EPSolar/1/ctemp:state:default" }

     Number EPSolar_BattVolt "Battery Voltage [%.2f V]" { mqtt="<[mymosquitto:EPSolar/1/bvoltage:state:default" }
     Number EPSolar_BattRemain "Battery Remaining [%.2f %]" { mqtt="<[mymosquitto:EPSolar/1/bremaining:state:default" }
     Number EPSolar_BattTemp "Battery Temp [%.2f °C]" { mqtt="<[mymosquitto:EPSolar/1/btemp:state:default" }

     Number EPSolar_LoadPower "Load Power [%.2f W]" { mqtt="<[mymosquitto:EPSolar/1/lpower:state:default" }
     Number EPSolar_LoadCurrent "Load Current [%.2f A]" { mqtt="<[mymosquitto:EPSolar/1/lcurrent:state:default" }

     Number EPSolar_PVVolt "PV Voltage [%.2f V]" { mqtt="<[mymosquitto:EPSolar/1/pvvoltage:state:default" }
     Number EPSolar_PVCurrent "PV Current [%.2f A]" { mqtt="<[mymosquitto:EPSolar/1/pvcurrent:state:default" }
     Number EPSolar_PVPower "PV Power [%.2f W]" { mqtt="<[mymosquitto:EPSolar/1/pvpower:state:default" }

     Number EPSolar_ChargeCurrent "Battery Charge Current [%.2f A]" { mqtt="<[mymosquitto:EPSolar/1/battChargeCurrent:state:default" }

     Number EPSolar_PVVoltMax "PV Voltage MAX (today) [%.2f V]" { mqtt="<[mymosquitto:EPSolar/1/stats_today_pv_volt_max:state:default" }
     Number EPSolar_PVVoltMin "PV Voltage MIN (today) [%.2f V]" { mqtt="<[mymosquitto:EPSolar/1/stats_today_pv_volt_min:state:default" }

This will store the results in these items. 

For full graphing, follow this tutorial: https://community.openhab.org/t/influxdb-grafana-persistence-and-graphing/13761

# Author

Darren Poulson <darren.poulson@gmail.com>



