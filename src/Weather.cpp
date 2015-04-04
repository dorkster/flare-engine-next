/*
Copyright © 2015 Wolfgang Pirker


*/

#include <cmath>
#include "CommonIncludes.h"
#include "SDLHardwareRenderDevice.h"
#include "SDLSoftwareRenderDevice.h"

#include "Weather.h"
#include "WeatherClimate.h"
#include "Utils.h"
#include "SharedGameResources.h"


//==============================================================================
WeatherCloud::WeatherCloud(FPoint poi, WeatherCloud::SizeType c_size,
WeatherCloud::IntensityType c_intensity)
	: size(c_size)
	, intensity(c_intensity)
{
};


WeatherCloud::~WeatherCloud(){
};

//------------------------------------------------------------------------------
void WeatherCloud::cleanup(WeatherCloud *cloud) {
    // FIXME: warnings
	free(cloud);
}

int WeatherCloud::getSize(WeatherCloud *cloud){
    return cloud->size;
}

int WeatherCloud::getIntensity(WeatherCloud *cloud){
    return cloud->intensity;
}

FPoint WeatherCloud::getCurrentPoint(WeatherCloud *cloud){
    return cloud->curr_p;
}

void WeatherCloud::fadeOff(WeatherCloud cloud, int dur){
    // TODO: fade off
}

ListWeatherCloud::ListWeatherCloud()
	: is_snow(false)
	, direction(2)
	, speed(1.0)
	, cycle_i(0)
	, cycle_max(0)
	, time_of_rain(0)
	, is_strong_rainfall(true)
	, state_is_initialized(false)
	, img_cloud(NULL)
	, img_rainfall(NULL)
	, spr_flake(NULL)
{
};

ListWeatherCloud::~ListWeatherCloud(){

};

ListWeatherCloud ListWeatherCloud::instance;

void ListWeatherCloud::setSnow(bool is_snow_a){
    is_snow = is_snow_a;
}
/* TODO: fog feature
void ListWeatherCloud::setFog(bool is_fog_a){
    is_fog = is_fog_a;
}
*/


void ListWeatherCloud::setWindDirection(FPoint p1, FPoint p2){
    ListWeatherCloud::direction = calcDirection(p1, p2);
}

int ListWeatherCloud::getWindDirection(){
    return ListWeatherCloud::direction;
}

void ListWeatherCloud::setWindForce(float speed_a){ // speed in approximate tiles per second
    speed=speed_a;
}

float ListWeatherCloud::getWindForce(){
    return speed;
}

void ListWeatherCloud::logicClouds() {
    //logicDebug();
    cycle_i+=1;
    moveClouds();

    if (cycle_i == cycle_max){
        clearUp();
        cycle_i = 0;
    }
    if (cycle_i > time_of_rain) rainfall();
}

bool ListWeatherCloud::logicClouds(int base_cloudiness_a, long cycle_max_a){
    int cloudiness;
    cycle_max = cycle_max_a + randBetween(-1000,1000); // TODO: needs adjustment
    cloudiness=base_cloudiness_a + randBetween(-20, 20);

    // FIXME: should this method really only create clouds if cloudslist is empty?
        // it could also make sense if cycle_max depends actually on one cloud itself
        // instead of all clouds, but it would make both classes more complicated
    if (cloud_list.empty()){

        const int half_max_cycle = cycle_max/2;
        createClouds(cloudiness);
        // rain variable: time_of_rain
          // determines after which time of the cycle it should
          // start to rain; no rain if time_of_rain>cycle_max;
          // depends both on cloudiness and some randomness
        time_of_rain = cycle_max * ( 10/cloudiness) + randBetween(0, half_max_cycle);
        // determine if it should be strong rainfall, let it depend on cloudiness and time_of_rain
        if ((cloudiness*time_of_rain/cycle_max>=16)) is_strong_rainfall = true;

        // instantiate RenderDevice Images
        if (is_snow){
            img_rainfall = render_device->loadImage("images/weather/snow_transparent.png",
                "Couldn't load snow image!", false);
        }
        else{
            img_rainfall = render_device->loadImage("images/weather/rain.png",
                "Couldn't load rain image!", false);
        }
        if (img_rainfall){
            spr_flake = img_rainfall->createSprite();
            //img_rainfall->unref();
        }


        // TODO: img_cloud


    }
    else {
        clearUp();
        return false;
    }
    return true;
}

void ListWeatherCloud::renderClouds(){

    //if (cycle_i > time_of_rain) renderRainfall(); // FIXME: think about merging renderRainfall and rainfall()
    renderRainfall(); // FIXME: remove this line, uncomment previous
}

ListWeatherCloud* ListWeatherCloud::getInstance() { return &ListWeatherCloud::instance; }

WeatherCloud ListWeatherCloud::get_first_cloud(){
    // Note: only use this method if cloud_list.empty() is false
    return cloud_list.front();
}

bool ListWeatherCloud::remove_first_cloud(){
    if (cloud_list.empty()) return false;
    cloud_list.pop_front();
    return true;
}

void ListWeatherCloud::createClouds(int cloudiness){
    WeatherCloud::SizeType size;
    WeatherCloud::IntensityType  intensity;
    std::list<WeatherCloud>::iterator it=cloud_list.begin();
    int i=0;

    //std::cout<<"cloudiness value: " << cloudiness << std::endl; // TODO: should be removed

    // Note: to make it easier no randomness when placing clouds,
      // could be optimized later...
      // assumes max. of 6 clouds!
    const float X_CORDS_ARR[] = {30.5, 490.3, 5.4, 880.4, 542.5, 992.5};
    const float Y_CORDS_ARR[] ={480.4, 725.5, 300.2, 850.2, 900.5, 548.5};

    while (cloudiness>4){ // create clouds // FIXME: adjust value perhaps

        if (i >= MAX_NUMBER_OF_CLOUDS-1) break;
        size=WeatherCloud::SizeType(randBetween(0,2));
        intensity=WeatherCloud::IntensityType(randBetween(0,2));
        cloud_list.insert(it, WeatherCloud(FPoint(X_CORDS_ARR[i],Y_CORDS_ARR[i]), size, intensity));
        //std::cout << "create Cloud: with size " << size << "; with intensity: " << intensity << std::endl; // TODO: should be removed
        cloudiness-=(size+intensity+2)*3;
        i+=1;
        it++;
    }
}

void ListWeatherCloud::moveClouds(){

}

void ListWeatherCloud::clearUp(){ // affects both: clouds and rainfall
    int dur=120;

    if (!cloud_list.empty()) return;
    WeatherCloud cloud=get_first_cloud();
    while (true){
        WeatherCloud::fadeOff(cloud, dur);
        if (!remove_first_cloud()) break;
        cloud=get_first_cloud();
    }
    render_device->freeImage(img_rainfall);
    state_is_initialized = false;
}

void ListWeatherCloud::rainfall(){
    if (ListWeatherCloud::is_snow){
        snow();
    }
    else rain();
}

void ListWeatherCloud::snow(){

}

void ListWeatherCloud::rain(){

}

void ListWeatherCloud::logicDebug(){
    if (cycle_i==1){
        std::cout<<"time of rain: " << time_of_rain << std::endl;
    }
    if (cycle_i==time_of_rain){
        std::cout<<"time rainfall starts" << std::endl;
    }
    if (cycle_i==cycle_max) {
        std::cout<<"time cycle ended" << std::endl;
        std::cout<<"clouds should fade off now..." << std::endl;
    }

}

void ListWeatherCloud::renderRainfall(){
    if (ListWeatherCloud::is_snow){
        renderSnow();
    }
    else renderRain();
}

void ListWeatherCloud::renderSnow(){
	if (!img_rainfall)
		return;

    int w = img_rainfall->getWidth();
    int h = img_rainfall->getHeight();
    int nr = 0;
    int i = 0;
    int j = 0;
    int r3 = 1; // changes type
    int r4 = 2;// changes offset
    int change_after = 14; // in frames
                                // Note: if there is ever a wind force implemented, it could be
                                // considered to depend this value upon it: more wind -> faster changes
    const int RADIUS = 20;
    int density = 3; // best values: 2, 3
    Rect screen_size = render_device->getContextSize();
    Point p;

    if (is_strong_rainfall){
        density = 2;
        change_after = 10;
    }

    // TODO: instead of the flocks falling from the top to the bottom a fade in /
      // fade out effect in between would surely look better

    // initialize the flake_state Array, if it isn't yet
    if (!state_is_initialized){

        state_is_initialized = true;
        while (nr < 48){
            r3 = randBetween(0,6);
            r4 = randBetween(-4,4);
            flake_state[nr][0] = r3; // type
            flake_state[nr][1] = mapr->cam.x; // x
            flake_state[nr][2] = mapr->cam.y; // y
            flake_state[nr][3] = r4;
            flake_state[nr][4] = 0;
            flake_state[nr][5] = 0;
            nr+=1;
        }

    }

    i = -2*RADIUS;
    j = -2*RADIUS;

    while(j < RADIUS){
        if (nr>47) nr=0;
        // update of position info should be rather slow...
          // the snowflakes appear to move with the character if true
        if (cycle_i % (change_after * 8) == 0) {
            flake_state[nr][1] = mapr->cam.x;
            flake_state[nr][2] = mapr->cam.y;
        }
        spr_flake->setClipW(8);
        spr_flake->setClipX(flake_state[nr][0]*9);

        // TODO: take into account wind direction (variable 'direction')
        p = map_to_screen(flake_state[nr][1] + i, flake_state[nr][2] + j, mapr->cam.x, mapr->cam.y);
        if (cycle_i % change_after == 0) {
            flake_state[nr][4] = flake_state[nr][4] + randBetween(-1,1);
            flake_state[nr][5] = flake_state[nr][5] + randBetween(-1,2);
        }

        spr_flake->setOffset(flake_state[nr][3],0);
        spr_flake->setDestX(p.x + flake_state[nr][4]);
        spr_flake->setDestY(p.y + flake_state[nr][5] % screen_size.h);

        render_device->render(spr_flake);
        i+=density;
        if (i>RADIUS){
            i= -2*RADIUS;
            j+=density;
        }
        nr+=1;
    }
    render_device->render(spr_flake);
}

void ListWeatherCloud::renderRain(){


}

WeatherManager::WeatherManager()
	: initialized(false)
	, list_weather_cloud(NULL)
{
    WeatherClimate *klima = WeatherClimate::getInstance();
    enabled = klima->getEnabled();
};

WeatherManager::~WeatherManager(){
};

//------------------------------------------------------------------------------
void WeatherManager::init(){
    //ListWeatherCloud *list_weather_cloud;
    WeatherClimate *klima = WeatherClimate::getInstance();
    SeasonType season_type = SeasonType(klima->getSeason());
    HumidityType humidity_type = HumidityType(klima->getHumidity());

    int cloudiness = 0;
    int fogginess = 20;
    int wind_direction = 2;
    float wind_speed = 1.0;
    bool is_snow = false;
    long cycle_max = 40000; // TODO: should be influenced by WeatherClimate settings

    //if (!enabled) return; // better check outside of this method

    if (season_type==klima->WINTER) is_snow=true;
    // Note: Likeliness of rain and clouds is not ONLY influenced by the
        // WeatherClimate setting, but is partly also random
        // the random part of both cloudiness and cycle_max is done in
        // ListWeatherClouds
    cloudiness = 10;
    // TODO: wind variables changes
      // ...
    if (humidity_type==WeatherClimate::NORMAL){
        cloudiness+=12;
    }
    else if(humidity_type==WeatherClimate::WET){
        cloudiness+=24;
    } // else DRY, stays
    if (season_type==WeatherClimate::SUMMER){ // dryer in the summer
        cloudiness-=14;
    }
    else if (season_type==WeatherClimate::AUTUNM){
        cloudiness+=10;
        fogginess = 50; // TODO, fog
    }
    // ListWeatherCloud mustn't use WeatherClimate!
    list_weather_cloud = ListWeatherCloud::getInstance();
    list_weather_cloud->setSnow(is_snow);
    list_weather_cloud->logicClouds(cloudiness, cycle_max);
}

void WeatherManager::logic(){
    if (!initialized) {
        WeatherManager::init();
        initialized=true;
        return;
    }
    list_weather_cloud->logicClouds();

}

void WeatherManager::render(){
    //ListWeatherCloud *list_weather_cloud; // TODO: use list_weather_cloud as class variable!
    //list_weather_cloud = ListWeatherCloud::getInstance();

    if (!initialized) return;
    list_weather_cloud->renderClouds();
}

bool WeatherManager::getEnabledFlag(){
    return enabled;
}


//WeatherManager* WeatherManager::getInstance() { return &WeatherManager::instance; }
