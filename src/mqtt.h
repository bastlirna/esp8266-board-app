#ifndef __MQTT_H
#define __MQTT_H

#include <PubSubClient.h>

#define MQTT_OFFLINE_MSG "OFFLINE"
#define MQTT_ONLINE_MSG "ONLINE"

#define MQTT_TOPIC_STATUS "$status"
#define MQTT_TOPIC_VERSION "$fw/version"
#define MQTT_TOPIC_BUILD "$fw/build"
#define MQTT_TOPIC_UPTIME "$uptime"

typedef struct _MQTTSubscribedTopics {
    char *name;
    bool global;
    void *attr;
    void (*cb) (char*, void *attr);
    struct _MQTTSubscribedTopics *next;
}MQTTSubscribedTopics;

class MQTT {
    public:
    MQTT(PubSubClient *_client) : client(_client), subscribedTopics(0), lastLoopRun(0) { }

    void init(); 
    void publish(char *topic, char *message);
    void publish(char *topic, char *message, bool retain);
    void publish(char *topic, uint32_t value);
    void publish(char *topic, uint32_t value, bool retain);

    void connect();
    void loop();

    void callback(char* topic, byte* payload, unsigned int length);

    void subscribe(char *topic, void (*cb) (char*, void *attr), void *attr, bool global);
    void subscribe(char *topic, void (*cb) (char*, void *attr), void *attr);

    void setLatestFw(char *fw);
    void setFwUrl(char *url);
    void checkUpdate();

    private:
    void client_id(char *out);
    void sendSubscribe();
    void calc_topic(char *topic, char *out);
    void calc_global_topic(char *topic, char *out);
    void update();

    PubSubClient *client;
    MQTTSubscribedTopics *subscribedTopics;
    uint32_t lastLoopRun;

    char *latestFw = 0;
    char *fwUrl = 0;

};

#endif
