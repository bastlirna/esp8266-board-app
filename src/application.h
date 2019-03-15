#ifndef __APPLICATION_H
#define __APPLICATION_H

#include <espboard-hal.h>
#include "mqtt.h"
#include "settings.h"

class Application {
    public:
    Application(ESPBoard::HAL *_hal, MQTT *_mqtt) : hal(_hal), mqtt(_mqtt), measureInterval(APP_INTERVAL), lastMeasure(0) {}
    void init();
    void loop();
    void setUnits();
    void setInterval(uint32_t interval);

    private:
    ESPBoard::HAL *hal;
    MQTT *mqtt;
    uint32_t measureInterval;
    uint32_t lastMeasure;
};



#endif
