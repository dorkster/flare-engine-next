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

const int MAX_NUMBER_OF_CLOUDS=6;
const int RENDER_CHANGE_AFTER=8;
const int RADIUS = 20; // area of weather around hero

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


class ListWeatherCloud{

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
        int flake_state[48][4]; // stores info about type and x,y cordinates, offset
                                // 0: type, 1: offset,
                                // 2: rand val delta x, 3: rand val delta y

		int cloud_state[MAX_NUMBER_OF_CLOUDS][6]; // stores info about location of clouds
								 // 0: size, 1: offset, 2: rand val dx, 3 rand val dy
								 // 4, 5: old values of mapr->cam
        bool flakes_arr_initialized;
        bool clouds_arr_initialized;

        int cloud_distance;

        Point p_orientation;
        Point p_flakes;


        Image *img_cloud;
        Image *img_rainfall;
        Sprite *spr_flake;

        void createClouds(int cloudiness);

		void logicDebug();

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
};

#endif
