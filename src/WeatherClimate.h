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

class WeatherClimate{

	private:
        static WeatherClimate instance;
        bool enabled;
		int season;
		int humidity;

        // Initialize WeatherClimate
		bool init();
		void cleanup();

    protected:
        enum SeasonType {
            WINTER = 0, // -> Rainfall is snow
            SPRING = 1,
            SUMMER = 2, // -> mostly dry weather
            AUTUNM = 3  // -> more fog likelyness
        };

        enum HumidityType {
                DRY = 0,
                NORMAL = 1,
                WET = 2
        };


    public:
        WeatherClimate();

        int getEnabled();
        void changeEnabledBool();

        int getSeason();
		int getHumidity();

        static WeatherClimate* getInstance();
};

#endif
