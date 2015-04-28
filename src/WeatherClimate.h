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

#ifndef WEATHERCLIMATE_H
#define WEATHERCLIMATE_H

enum SeasonType {
    WEATHER_SEASON_WINTER = 0, // -> Rainfall is snow
    WEATHER_SEASON_SPRING = 1,
    WEATHER_SEASON_SUMMER = 2, // -> mostly dry weather
    WEATHER_SEASON_AUTUNM = 3  // -> more fog likelyness
};

enum HumidityType {
	WEATHER_HUMIDITY_DRY = 0,
	WEATHER_HUMIDITY_NORMAL = 1,
	WEATHER_HUMIDITY_WET = 2
};

class WeatherClimate{

	private:
        bool enabled;
		int season;
		int humidity;

    public:
        WeatherClimate();

        int getEnabled();
        void changeEnabledBool();

        int getSeason();
		int getHumidity();
};

#endif
