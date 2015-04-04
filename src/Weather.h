/*
Copyright © 2015 Wolfgang Pirker


*/


#ifndef WEATHERRAINFALL_H
#define WEATHERRAINFALL_H

#include <list>
#include "CommonIncludes.h" // --> includes several common-used headers, such as iostream, SDL2 etc.
#include "SharedResources.h" // --> includes loadIcons
#include "UtilsMath.h" // --> includes random number function and rand chance function
#include "WeatherClimate.h"
//#include "Entity.h"

const int MAX_NUMBER_OF_CLOUDS=4; // atm shouldnt be higher than 6!
                                    // see below ListWeatherCloud::createClouds()
//extern const int directionDeltaX;
//extern const int directionDeltaY;

class WeatherCloud{
	private:
        //static WeatherCloud instance;
        int size;
        int intensity;
        //int direction;
        //float speed;
        FPoint curr_p;

		// Free up resources
		void cleanup(WeatherCloud *cloud);


    public:
        enum SizeType {
        // influences rain (amount and if it rains)
            SMALL = 0,
            MID = 1,
            BIG = 2
        };

        enum IntensityType {
        // influences rain (amount and if)
            THIN = 0,
            MODEST = 1,
            THICK = 2
        };

        WeatherCloud();
        WeatherCloud(FPoint poi, SizeType c_size, IntensityType c_intensity);
		~WeatherCloud();

		static int getSize(WeatherCloud *cloud);
		static int getIntensity(WeatherCloud *cloud);
		static FPoint getCurrentPoint(WeatherCloud *cloud);
		static void fadeOff(WeatherCloud cloud, int fade_dur);
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
        bool state_is_initialized;

        Image *img_cloud;
        Image *img_rainfall;
        Sprite *spr_flake;
        //std::list<Sprite> spr_rainfall_list;

        //virtual Renderable getRender();

        WeatherCloud get_first_cloud();
        bool remove_first_cloud();

        void createClouds(int cloudiness); // base.. depends on settings
                                                                    // TODO: cycle setting
        void moveClouds(); // only logic!
        void clearUp(); // only logic!
        void rainfall(); // only logic
        void snow(); // only logic
		void rain(); // only logic

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
