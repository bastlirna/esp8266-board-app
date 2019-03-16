#include <Arduino.h>
#include <espboard-hal.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "application.h"
#include "config.h"
#include "settings.h"
#include "mqtt.h"

using namespace ESPBoard;

static HAL hal;
static WiFiClient espClient;
static PubSubClient client(espClient);
static MQTT mqtt(&client);
static Application app(&hal, &mqtt);
static Config conf;

static uint32_t wifiConnection = 0;
static uint32_t mqttConnection = 0;


static void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    mqtt.callback(topic, payload, length);
}

void setup() {
    
    Serial.begin(115200);
    Serial.println("");
    Serial.println("");

    Serial.print("Project version: ");
    Serial.println(BUILD_VERSION);

    conf.init();
    conf.print();

    mqtt.init();

    hal.init();
    app.init();

    client.setServer(conf.s.mqtt_host, *conf.s.mqtt_port);
    client.setCallback(mqtt_callback);


}

void loop() {
    
    if (WiFi.status() != WL_CONNECTED) {
        mqttConnection = 0;
        if (wifiConnection == 0 || (millis() - wifiConnection) > WIFI_RECONNECT_TIMEOUT) {
            wifiConnection = millis();
            conf.wifiConnect();
        }
        return;
    } 

    if (!client.connected()) {
        if (mqttConnection == 0 || (millis() - mqttConnection) > MQTT_RECONNECT_TIMEOUT) {
            mqttConnection = millis();
            mqtt.connect();
        }
        return;
    }
    
    client.loop();
    mqtt.loop();
    app.loop();

}