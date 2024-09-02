#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "device_config.h"
#include "esp_mac.h"  // required - exposes esp_mac_type_t values

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
long interval = 90000;
char msg[50];
int value = 0;

const int oneWireBus = 15;  
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

int counter;
char buffer[7];




void setup() {
    Serial.begin(115200);

    sensors.begin();
    
    WiFiManager wm;
    //wm.resetSettings(); // wipe settings

    bool res;
    res = wm.autoConnect(ESP_HOSTNAME,ESP_PWD); // password protected ap
    if(!res) {
        Serial.println("Failed to connect");
        ESP.restart();
    } 
    else {
        
        Serial.println("connected...yeey :)");
        client.setServer(MQTT_HOST, MQTT_PORT);
    }
}

void reconnect() {  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");    
    if (client.connect("ESP32Client")) {
      Serial.println("connected");      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttPubTemperature(){
    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    char tempString[8];
    dtostrf(temperatureC, 1, 2, tempString);
    client.publish(TOPIC_PUB, tempString);
    Serial.println("published");     
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > interval)
  {
    lastMsg = now;
    mqttPubTemperature();
  }
}

String getDefaultMacAddress() {

  String mac = "";

  unsigned char mac_base[6] = {0};

  if (esp_efuse_mac_get_default(mac_base) == ESP_OK) {
    char buffer[18];  // 6*2 characters for hex + 5 characters for colons + 1 character for null terminator
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
    mac = buffer;
  }

  return mac;
}