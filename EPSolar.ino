#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <SimpleTimer.h>
#include <ModbusMaster.h>
#include <PubSubClient.h>
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

int timerTask2, timerTask3;
float ctemp, bvoltage, battChargeCurrent, btemp, bremaining, lpower, lcurrent, pvvoltage, pvcurrent, pvpower;
uint8_t result;


const char* ssid = "60 Chequers Avenue AP1";
const char* password = "trial3211";
const char* mqtt_server = "192.168.6.46";

#define OTA_HOSTNAME                    "SOLAR-CHARGE-MONITOR-1"

WiFiClient espClient;
PubSubClient client(espClient);

// this is to check if we can write since rs485 is half duplex
bool rs485DataReceived = true;

ModbusMaster node;
SimpleTimer timer;

char buf[100];

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
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  // Modbus slave ID 1
  node.begin(1, Serial);
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
  client.setServer(mqtt_server, 1883);


  timerTask2 = timer.setInterval(10000, doRegistryNumber);
  timerTask3 = timer.setInterval(10000, nextRegistryNumber);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("EPSolar1")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("EPSolar/1/loop1", "Reconnect");
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void doRegistryNumber() {
  Registries[currentRegistryNumber]();
}

void AddressRegistry_3100() {
  char buf[50];
  result = node.readInputRegisters(0x3100, 7);
  if (result == node.ku8MBSuccess)
  {
    client.publish("EPSolar/1/loop1", "result");
   
    dtostrf((node.getResponseBuffer(0x11) / 100.0f), 6, 3, buf );
    client.publish("EPSolar/1/temp1", buf);
    
    dtostrf((node.getResponseBuffer(0x04) / 100.0f), 6, 3, buf );
    client.publish("EPSolar/1/bvoltage", buf);

    lpower = ((long)node.getResponseBuffer(0x0F) << 16 | node.getResponseBuffer(0x0E)) / 100.0f;
    dtostrf(lpower, 6, 3, buf);
    client.publish("EPSolar/1/lpower", buf);

    lcurrent = (long)node.getResponseBuffer(0x0D) / 100.0f;
    dtostrf(lcurrent, 6, 3, buf );
    client.publish("EPSolar/1/lcurrent", buf);

    pvvoltage = (long)node.getResponseBuffer(0x00) / 100.0f;
    dtostrf(pvvoltage, 6, 3, buf );
    client.publish("EPSolar/1/pvvoltage", buf);

    pvcurrent = (long)node.getResponseBuffer(0x01) / 100.0f;
    dtostrf(pvcurrent, 6, 3, buf );
    client.publish("EPSolar/1/pvcurrent", buf);   

    pvpower = ((long)node.getResponseBuffer(0x03) << 16 | node.getResponseBuffer(0x02)) / 100.0f;
    dtostrf(pvpower, 6, 3, buf );
    client.publish("EPSolar/1/pvpower", buf);
    
    battChargeCurrent = (long)node.getResponseBuffer(0x05) / 100.0f;
    dtostrf(battChargeCurrent, 6, 3, buf );
    client.publish("EPSolar/1/battChargeCurrent", buf);

  } else {
    rs485DataReceived = false;
  }
}

void AddressRegistry_311A() {
  char buf[10];
  result = node.readInputRegisters(0x311A, 2);
  if (result == node.ku8MBSuccess)
  {
    bremaining = node.getResponseBuffer(0x00) / 1.0f;
    dtostrf(bremaining, 6, 3, buf );
    client.publish("EPSolar/1/bremaining", buf);
    
    btemp = node.getResponseBuffer(0x01) / 100.0f;
    dtostrf(btemp, 6, 3, buf );
    client.publish("EPSolar/1/btemp", buf);
    
  } else {
    rs485DataReceived = false;
  }
}

void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  timer.run();
}

