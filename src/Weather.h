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

#ifndef WEATHERRAINFALL_H
#define WEATHERRAINFALL_H

#include <list>
#include "CommonIncludes.h"
#include "SharedResources.h"
#include "UtilsMath.h"
#include "WeatherClimate.h"

const int MAX_NUMBER_OF_CLOUDS=8;
const int RENDER_CHANGE_AFTER=8;

class WeatherCloud{
	private:
        Sprite *spr_cloud;
    public:
        enum SizeType {
            SMALL = 0,
            MID = 1,
            BIG = 2
        };

        WeatherCloud();
        WeatherCloud(SizeType c_size, int shape);
		~WeatherCloud();

		Sprite* getSprite();
};


class ListWeatherCloud{ // Container for weather clouds
                    // makes them rain, snow, disappear...
    public:
        ListWeatherCloud();
        ~ListWeatherCloud();
        void setSnow(bool is_snow);
        // void setFog(bool is_fog); TODO

        void logicClouds();
        bool logicClouds(int base_cloudiness, long base_cycles);
        void renderClouds();

        void setWindDirection(FPoint p1, FPoint p2);
        int getWindDirection();
        void setWindForce(float speed);
        float getWindForce();

        static ListWeatherCloud* getInstance();

    private:
        static ListWeatherCloud instance;
        std::list<WeatherCloud> cloud_list;
        bool is_snow;
        // bool is_fog; TODO
        int direction;
        float speed;
        long cycle_i;
        long cycle_max; // frames until the weather changes
        long time_of_rain;
        bool is_strong_rainfall; // two types of rainfall: light or strong
                                    // TODO: more adjustment possibilities
        int flake_state[48][6]; // stores info about type and x,y cordinates, offset
                                // 0: type, 1: cam.x, 2: cam.y, 3: offset,
                                // 4: rand val delta x, 5: rand val delta y

		int cloud_state[MAX_NUMBER_OF_CLOUDS][5]; // stores info about location of clouds
								 // 0: p.x, 1: p.y, 2: offset, TODO ==> needs CORRECTION
								 // 3: rand val delta x, 4: rand val delta y

        bool state_is_initialized;


        Image *img_cloud;
        Image *img_rainfall;
        Sprite *spr_flake;
        Image *weather_surface;
        Sprite *spr_weather;

        //WeatherCloud* getFirstCloud();
        //bool removeFirstCloud();

        void createClouds(int cloudiness); // base.. depends on settings
                                                                    // TODO: cycle setting
		Point findValidPos(uint radiant=32);
        Point moveCloud(int number); // only logic!

        /*void rainfall(); // only logic
        void snow(); // only logic
		void rain(); // only logic*/


		void logicDebug();

		// render functions:
		void renderRainfall();
		void renderSnow();
		void renderRain();
};


class WeatherManager : public WeatherClimate {
	private:
        bool enabled;
        bool initialized;
        ListWeatherCloud *list_weather_cloud;
        void init();

    public:
        WeatherManager();
		~WeatherManager();

		void logic();
		void render();
		bool getEnabledFlag();
		//void requestNewInitialization(); --> not sure if there was a need for that


};

#endif
