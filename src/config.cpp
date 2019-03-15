#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>

#include "config.h"

static void populate_settings(uint8_t* buffer, settings_s* s){
    s->ssid = (char*) (buffer);
    s->password = (char*) (buffer + 50);
    s->mqtt_host = (char*) (buffer + 100);
    s->mqtt_port = (uint16_t*) (buffer + 150);
}

void Config::init()
{
    EEPROM.begin(300);
    populate_settings(EEPROM.getDataPtr(), &s);
}

#define PRINT(name, value) Serial.print(name); Serial.print(": "); Serial.println(value);
void Config::print()
{
    PRINT("ssid", s.ssid);
    PRINT("password", "********");
    PRINT("mqtt_host", s.mqtt_host);
    PRINT("mqtt_port", *s.mqtt_port);
}

void Config::setUpWifi() 
{
    WiFi.begin(s.ssid, s.password);
}

void Config::wifiConnect() 
{
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(s.ssid);

    setUpWifi();

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

