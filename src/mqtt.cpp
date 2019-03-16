#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266httpUpdate.h>

#include "mqtt.h"
#include "settings.h"

void MQTT::client_id(char *out) 
{
    uint8_t mac[6];
    WiFi.macAddress(mac);
    sprintf(out, "ESPDemoBoard%02x%02x%02x", mac[3], mac[4], mac[5]);
}

void MQTT::calc_topic(char *topic, char *out) 
{
    uint8_t mac[6];
    WiFi.macAddress(mac);
    
    sprintf(out, "AD19/%02x%02x%02x/%s", mac[3], mac[4], mac[5], topic);
}

void MQTT::calc_global_topic(char *topic, char *out) 
{
    sprintf(out, "AD19/master/%s", topic);
}

void MQTT::checkUpdate()
{
    if (latestFw != 0 && strlen(latestFw) > 0 && strcmp(latestFw, BUILD_VERSION) != 0) {
        Serial.println("Firmware is not actual.");

        if (fwUrl != 0 && strlen(fwUrl) > 0 && strncmp(fwUrl, "https", 5) == 0) {
            Serial.print("Downloading new FW from ");
            Serial.println(fwUrl);

            update();
        }
    }
}

void MQTT::update()
{
    WiFiClientSecure client;

    if (fwUrl == 0) {
        return;
    }

    Serial.print("Flash started ... ");
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, fwUrl);
    
    if (ret != HTTP_UPDATE_OK) {
        Serial.println("failed");
        return;
    }
    
    Serial.println("done");
}

void MQTT::setLatestFw(char* fw) 
{
    if(latestFw != 0) {
        free(latestFw);
    }

    latestFw = (char*) malloc(strlen(fw) + 1);
    strcpy(latestFw, fw);
    Serial.print("Set latest FW version to ");
    Serial.println(latestFw);

    checkUpdate();
}

void MQTT::setFwUrl(char* url) 
{
    if(fwUrl != 0) {
        free(fwUrl);
    }

    fwUrl = (char*) malloc(strlen(url) + 1);
    strcpy(fwUrl, url);
    Serial.print("Set FW update url to ");
    Serial.println(fwUrl);
    
    checkUpdate();
}

static void cb_last_fw(char *data, void *attr) 
{
    MQTT *mqtt = (MQTT*)attr;
    mqtt->setLatestFw(data);
}

static void cb_fw_url(char *data, void *attr) 
{
    MQTT *mqtt = (MQTT*)attr;
    mqtt->setFwUrl(data);
}

void MQTT::init()
{
    subscribe("$fw/last", cb_last_fw, this, true);
    subscribe("$fw/url", cb_fw_url, this, true);
}

void MQTT::publish(char *topic, char *message)
{
    publish(topic, message, false);
}

void MQTT::publish(char *topic, char *message, bool retain)
{
    if (!client->connected())
    {
        return;
    }

    char fullTopic[64];
    calc_topic(topic, fullTopic);

    Serial.print("Publish to [");
    Serial.print(fullTopic);
    Serial.print("] message '");
    Serial.print(message);
    Serial.println("'");
    client->publish(fullTopic, message, retain);
}

void MQTT::publish(char *topic, uint32_t value)
{
    publish(topic, value, false);
}

void MQTT::publish(char *topic, uint32_t value, bool retain)
{
    char str[32];
    sprintf(str, "%u", value);
    publish(topic, str, retain);
}

void MQTT::sendSubscribe() 
{
    char tmp[64];
    MQTTSubscribedTopics *st = subscribedTopics;

    while(st != 0) {
        if (st->global) {
            calc_global_topic(st->name, tmp);
        } else {
            calc_topic(st->name, tmp);
        }
        Serial.print("Subscribe to [");
        Serial.print(tmp);
        Serial.println("]");
        client->subscribe(tmp);
        st = st->next;
    }
}

void MQTT::callback(char* topic, byte* payload, unsigned int length)
{
    char tmp[64];
    char ptext[MQTT_PAYLOAD_MAX_LENGTH + 1];
    MQTTSubscribedTopics *st = subscribedTopics;

    while(st != 0) {
        if (st->global) {
            calc_global_topic(st->name, tmp);
        } else {
            calc_topic(st->name, tmp);
        }
       
        if (strcmp(tmp, topic) == 0) {
       
            if (length > MQTT_PAYLOAD_MAX_LENGTH) {
                length = MQTT_PAYLOAD_MAX_LENGTH;
            }
            memcpy(ptext, payload, length);
            ptext[length] = 0;

            Serial.print("Recieved message on [");
            Serial.print(topic);
            Serial.print("] with data '");
            Serial.print(ptext);
            Serial.println("'");
            
            (st->cb) (ptext, st->attr);
            return;
        }
        
        st = st->next;
    }
}

void MQTT::connect()
{
    char clientId[32];
    char topic[64];
    
    
    if (!client->connected()) {
        Serial.print("Attempting MQTT connection using id='");
        client_id(clientId);
        Serial.print(clientId);
        Serial.print("' ...");
        
        calc_topic(MQTT_TOPIC_STATUS, topic);
        if (client->connect(clientId, topic, 0, true, MQTT_OFFLINE_MSG)) {
            Serial.println(" connected");

            sendSubscribe();
            publish(MQTT_TOPIC_STATUS, MQTT_ONLINE_MSG, true);
            publish(MQTT_TOPIC_VERSION, BUILD_VERSION, true);
            publish(MQTT_TOPIC_BUILD, __DATE__ " " __TIME__, true);

        } else {
            Serial.print(" failed, rc=");
            Serial.print(client->state());
            Serial.println("");
        }
    }
}

void MQTT::subscribe(char *topic, void (*cb) (char*, void *), void *attr) 
{
    subscribe(topic, cb, attr, false);
}

void MQTT::subscribe(char *topic, void (*cb) (char*, void *), void *attr, bool global)
{

    MQTTSubscribedTopics *st = (MQTTSubscribedTopics*) malloc(sizeof(MQTTSubscribedTopics));
    st->name = (char*) malloc(strlen(topic) + 1);
    strcpy(st->name, topic);
    st->global = global;
    st->cb = cb;
    st->attr = attr;
    st->next = subscribedTopics;
    subscribedTopics = st;

    if (client->connected()) {
        sendSubscribe();
    }
}

void MQTT::loop()
{
    if (lastLoopRun == 0 || (millis() - lastLoopRun) > MQTT_LOOP_RUN_TIMEOUT) {
        lastLoopRun = millis();
        publish(MQTT_TOPIC_UPTIME, millis() / 1000);
    }
    
}