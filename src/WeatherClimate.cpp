/*
Copyright © 2015 Wolfgang Pirker


*/

#include "WeatherClimate.h"

WeatherClimate WeatherClimate::instance;

//==============================================================================
WeatherClimate::WeatherClimate() {
    init();
}

bool WeatherClimate::init() {
    enabled=true;
    season=0;
    humidity=2;
    return true;
}

//------------------------------------------------------------------------------
void WeatherClimate::cleanup() {

}

// TODO: make use of a settings file e.g. engine/weather.txt
int WeatherClimate::getSeason(){
    return season;
}

int WeatherClimate::getHumidity(){
    return humidity;
}

int WeatherClimate::getEnabled(){
    return enabled;
}

void WeatherClimate::changeEnabledBool(){
    WeatherClimate::enabled = !WeatherClimate::enabled;
}

WeatherClimate* WeatherClimate::getInstance() { return &WeatherClimate::instance; }

