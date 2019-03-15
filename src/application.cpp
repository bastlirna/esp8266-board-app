#include <Arduino.h>
#include "application.h"
#include "settings.h"
#include "utils.h"


static void cb_units(char *data, void *attr) 
{
    Application *app = (Application*)attr;
    app->setUnits();
}

static void cb_interval(char *data, void *attr) 
{
    Application *app = (Application*)attr;
    if (isInt(data)) {
        app->setInterval(atoi(data) * 1000);    
    } else {
        Serial.println("Interval isn't integer");
    }
}

void Application::init()
{
    mqtt->subscribe("settings/unit", cb_units, this);
    mqtt->subscribe("settings/interval", cb_interval, this);
}


void Application::setInterval(uint32_t interval)
{
    if (interval > APP_MAX_INTERVAL) {
        interval = APP_MAX_INTERVAL;
    }

    if (interval < APP_MIN_INTERVAL) {
        interval = APP_MIN_INTERVAL;
    }

    Serial.print("Set interval to ");
    Serial.print(interval);
    Serial.println();
    measureInterval = interval;
}

void Application::setUnits()
{

}

void Application::loop()
{   
    char tmp[32];
    if (lastMeasure == 0 || (millis() - lastMeasure) > measureInterval) {
        float temp = hal->temperature();
        sprintf(tmp, "%0.2f", temp);
        mqtt->publish("temperature", tmp);
        lastMeasure = millis();
    }
}

