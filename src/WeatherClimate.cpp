/*
Copyright © 2015 Wolfgang Pirker

This file is part of FLARE.

FLARE is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

FLARE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
FLARE.  If not, see http://www.gnu.org/licenses/
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

// TODO: make use of a settings from files
// settings to be used in maps setting files: season, humidity, enabled
// settings to be used in engine/weather.txt: enabled
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

