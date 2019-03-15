#ifndef __CONFIG_H
#define __CONFIG_H

struct settings_s {
    char* ssid;
    char* password;
    char* mqtt_host;
    uint16_t* mqtt_port;
};

class Config {
    public:

    void init();
    void print();
    void setUpWifi();
    void wifiConnect();

    settings_s s;
};

#endif
