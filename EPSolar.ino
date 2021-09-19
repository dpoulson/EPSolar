/*

    Copyright (C) 2017 Darren Poulson <darren.poulson@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <SimpleTimer.h>
#include <ModbusMaster.h>
#include <LeifHomieLib.h>
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

int timerTask2, timerTask3;
float bvoltage, bcurrent, bpower, btemp, bremaining, lvoltage, lcurrent, lpower, pvvoltage, pvcurrent, pvpower;
uint8_t result;

WiFiClient espClient;

HomieDevice homie;


HomieProperty * pPropbvoltage=NULL;
HomieProperty * pPropbcurrent=NULL;
HomieProperty * pPropbpower=NULL;
HomieProperty * pPropbremaining=NULL;

HomieProperty * pProplvoltage=NULL;
HomieProperty * pProplcurrent=NULL;
HomieProperty * pProplpower=NULL;

HomieProperty * pProppvvoltage=NULL;
HomieProperty * pProppvcurrent=NULL;
HomieProperty * pProppvpower=NULL;


// this is to check if we can write since rs485 is half duplex
bool rs485DataReceived = true;

ModbusMaster node;
SimpleTimer timer;

char buf[10];
String value;


// tracer requires no handshaking
void preTransmission() {}
void postTransmission() {}

// a list of the regisities to query in order
typedef void (*RegistryList[])();
RegistryList Registries = {
  AddressRegistry_3100,
  AddressRegistry_311A,
};
// keep log of where we are
uint8_t currentRegistryNumber = 0;

// function to switch to next registry
void nextRegistryNumber() {
  currentRegistryNumber = (currentRegistryNumber + 1) % ARRAY_SIZE( Registries);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.hostname(OTA_HOSTNAME);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  // Modbus slave ID 1
  node.begin(EPSOLAR_DEVICE_ID, Serial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(OTA_HOSTNAME);

  // No authentication by default
  //ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();


  HomieLibRegisterDebugPrintCallback([](const char * szText){
    Serial.printf("%s",szText);
    });
    {
      HomieNode * pNode=homie.NewNode();
      pNode->strID="properties";
      pNode->strFriendlyName="Properties";

      HomieProperty * pProp;
      
      pPropbvoltage=pProp=pNode->NewProperty();
      pProp->strFriendlyName="Battery Voltage";
      pProp->strID="bvoltage";
      pProp->strFormat="V";
      pProp->SetRetained(true);
      pProp->datatype=homieFloat;

      pPropbcurrent=pProp=pNode->NewProperty();
      pProp->strFriendlyName="Battery Charging Current";
      pProp->strID="bcurrent";
      pProp->strFormat="A";
      pProp->SetRetained(true);
      pProp->datatype=homieFloat;

      pPropbpower=pProp=pNode->NewProperty();
      pProp->strFriendlyName="Battery Charging Power";
      pProp->strID="bpower";
      pProp->strFormat="W";
      pProp->SetRetained(true);
      pProp->datatype=homieFloat;

      pPropbremaining=pProp=pNode->NewProperty();
      pProp->strFriendlyName="Battery Remaining";
      pProp->strID="bremaining";
      pProp->strFormat="%";
      pProp->SetRetained(true);
      pProp->datatype=homieFloat;

      pProppvvoltage=pProp=pNode->NewProperty();
      pProp->strFriendlyName="PV Voltage";
      pProp->strID="pvvoltage";
      pProp->strFormat="V";
      pProp->SetRetained(true);
      pProp->datatype=homieFloat;

      pProppvcurrent=pProp=pNode->NewProperty();
      pProp->strFriendlyName="PV Current";
      pProp->strID="pvcurrent";
      pProp->strFormat="A";
      pProp->SetRetained(true);
      pProp->datatype=homieFloat;

      pProppvpower=pProp=pNode->NewProperty();
      pProp->strFriendlyName="PV Power";
      pProp->strID="pvpower";
      pProp->strFormat="W";
      pProp->SetRetained(true);
      pProp->datatype=homieFloat;      

      pProplvoltage=pProp=pNode->NewProperty();
      pProp->strFriendlyName="Load Voltage";
      pProp->strID="lvoltage";
      pProp->strFormat="V";
      pProp->SetRetained(true);
      pProp->datatype=homieFloat;

      pProplcurrent=pProp=pNode->NewProperty();
      pProp->strFriendlyName="Load Current";
      pProp->strID="lcurrent";
      pProp->strFormat="A";
      pProp->SetRetained(true);
      pProp->datatype=homieFloat;

      pProplpower=pProp=pNode->NewProperty();
      pProp->strFriendlyName="Load Power";
      pProp->strID="lpower";
      pProp->strFormat="W";
      pProp->SetRetained(true);
      pProp->datatype=homieFloat;      
    }

  homie.strFriendlyName="EPSolar 1";
  homie.strID="EPsolar1";
  homie.strID.toLowerCase();

  homie.strMqttServerIP=MQTT_IP;
  homie.strMqttUserName=MQTT_USER;
  homie.strMqttPassword=MQTT_PASS;

  homie.Init();


  timerTask2 = timer.setInterval(10000, doRegistryNumber);
  timerTask3 = timer.setInterval(10000, nextRegistryNumber);
}

void doRegistryNumber() {
  Registries[currentRegistryNumber]();
}

void AddressRegistry_3100() {
  result = node.readInputRegisters(0x3100, 16);
  if (result == node.ku8MBSuccess)
  {
    bvoltage = (long)node.getResponseBuffer(0x04) / 100.0f;
    pPropbvoltage->SetValue(dtostrf(bvoltage, 2, 3, buf ));

    bcurrent = (long)node.getResponseBuffer(0x05) / 100.0f;
    pPropbcurrent->SetValue(dtostrf(bcurrent, 2, 3, buf ));

    bpower = ((long)node.getResponseBuffer(0x07) << 16 | node.getResponseBuffer(0x06)) / 100.0f;
    pPropbpower->SetValue(dtostrf(bpower, 2, 3, buf ));

    lvoltage = (long)node.getResponseBuffer(0x0C) / 100.0f;
    pProplvoltage->SetValue(dtostrf(lvoltage, 2, 3, buf ));
    
    lcurrent = (long)node.getResponseBuffer(0x0D) / 100.0f;
    pProplcurrent->SetValue(dtostrf(lcurrent, 2, 3, buf ));  
      
    lpower = ((long)node.getResponseBuffer(0x0F) << 16 | node.getResponseBuffer(0x0E)) / 100.0f;
    pProplpower->SetValue(dtostrf(lpower, 2, 3, buf));

    pvvoltage = (long)node.getResponseBuffer(0x00) / 100.0f;
    pProppvvoltage->SetValue(dtostrf(pvvoltage, 2, 3, buf ));

    pvcurrent = (long)node.getResponseBuffer(0x01) / 100.0f;
    pProppvcurrent->SetValue(dtostrf(pvcurrent, 2, 3, buf ));

    pvpower = ((long)node.getResponseBuffer(0x03) << 16 | node.getResponseBuffer(0x02)) / 100.0f;
    pProppvpower->SetValue(dtostrf(pvpower, 2, 3, buf ));

  } else {
    rs485DataReceived = false;
  }
}

void AddressRegistry_311A() {
  result = node.readInputRegisters(0x311A, 2);
  if (result == node.ku8MBSuccess)
  {
    bremaining = node.getResponseBuffer(0x00) / 1.0f;
    pPropbremaining->SetValue(dtostrf(bremaining, 2, 3, buf ));
    
  } else {
    rs485DataReceived = false;
  }
}


void loop() {
  ArduinoOTA.handle();
  timer.run();
  homie.Loop();
}
