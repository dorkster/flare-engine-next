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

#include <cmath>
#include "CommonIncludes.h"
#include "SDLHardwareRenderDevice.h"
#include "SDLSoftwareRenderDevice.h"

#include "Weather.h"
#include "WeatherClimate.h"
#include "Utils.h"
#include "SharedGameResources.h"


//==============================================================================
WeatherCloud::WeatherCloud(Point poi, WeatherCloud::SizeType c_size,
WeatherCloud::IntensityType c_intensity, int s)
	: curr_p(poi)
	, size(c_size)
	, intensity(c_intensity)
	, shape(s)
	//, to_be_rendered(true)
{
};


WeatherCloud::~WeatherCloud(){
};

//------------------------------------------------------------------------------
void WeatherCloud::cleanup(WeatherCloud *cloud) {
    // FIXME: warnings
	free(cloud);
}

int WeatherCloud::getSize(){
    return WeatherCloud::size;
}

int WeatherCloud::getIntensity(){
    return WeatherCloud::intensity;
}

Point WeatherCloud::getCurrentPoint(){
    return curr_p;
}

void WeatherCloud::setPoint(Point p){
	WeatherCloud::curr_p =p;
}

int WeatherCloud::getShape(){
	return WeatherCloud::shape;
}

bool WeatherCloud::reduceIntensity(){
	if (intensity==0) return false;
    intensity-=1;
    return true;
}

/*void WeatherCloud::setToBeRendered(bool flag){
	to_be_rendered = flag;
}*/

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
	, spr_small_cloud(NULL)
	, spr_mid_cloud(NULL)
	, spr_big_cloud(NULL)
{
};

ListWeatherCloud::~ListWeatherCloud(){
	if (!cloud_list.empty()){
		cloud_list.clear();
	}
	if (img_rainfall != NULL) render_device->freeImage(img_rainfall);
	if (img_cloud != NULL){
		render_device->freeImage(img_cloud);
		img_cloud->unref();
	}

	delete spr_big_cloud;
	delete spr_mid_cloud;
	delete spr_small_cloud;
	delete spr_flake;
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
    const int ACT_AFTER = 200; // frames
								// TODO: as setting in weather.txt perhaps
    int intervall;
    bool is_valid_pos;
	Point p;
	std::list<WeatherCloud>::iterator it=cloud_list.begin();
    WeatherCloud *cloud;
    cycle_i+=1;

    // iterate through all clouds...
    // move them and check position
    if (cycle_i % ACT_AFTER == 0){
		while (it != cloud_list.end()){
			cloud = &*it;
			p = cloud->getCurrentPoint();

			moveCloud();

			is_valid_pos = mapr->collider.is_valid_position(p.x, p.y, MOVEMENT_FLYING, false);
			if (!is_valid_pos){ // clouds on invalid positions due to maps change
				p = findValidPos();
				// FIXME: case if no valid position was found is not handled
				// endless loop danger possible
				cloud->setPoint(p);
			}
			it++;
		}
	}
    /*if (cycle_i >= time_of_rain){
		rainfall();
	}*/
	if (cycle_i >= cycle_max){
		intervall = randBetween(570,5700);
		clearUp(intervall);
		// rain should stop sooner or later
		is_strong_rainfall = false;
		time_of_rain += 4; // TODO: needs adjustment
	}
}

bool ListWeatherCloud::logicClouds(int base_cloudiness_a, long cycle_max_a){
    int cloudiness;
    cycle_max = cycle_max_a + randBetween(-5000,5000); // TODO: needs adjustment
    cloudiness=base_cloudiness_a + randBetween(-40, 40);

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
        if ((cloudiness*time_of_rain/cycle_max>=30)) is_strong_rainfall = true;

        // instantiate RenderDevice Images
        if (is_snow){
            img_rainfall = render_device->loadImage("images/weather/snow_transparent.png",
                "Couldn't load snow image!", false);
        }
        else{
            img_rainfall = render_device->loadImage("images/weather/rain.png",
                "Couldn't load rain image!", false);
        }
        img_cloud = render_device->loadImage("images/weather/clouds_gs.png",
				"Couldn't load cloud image!", false);
        if (img_rainfall){
            spr_flake = img_rainfall->createSprite();
            img_rainfall->unref();
        }
        if (img_cloud){
			spr_small_cloud = img_cloud->createSprite();
			spr_mid_cloud = img_cloud->resize(img_cloud->getWidth()*2,img_cloud->getHeight()*2)->createSprite();
			spr_big_cloud = img_cloud->resize(img_cloud->getWidth()*3,img_cloud->getHeight()*3)->createSprite();
            img_cloud->unref();
        }

        cycle_i = 0;
    }
    /*else { // could happen after map was changed
        // TODO: put clouds to new valid positions
        return false;
    }*/
    return true;
}

void ListWeatherCloud::renderClouds(){
	if ( (img_cloud) && (!cloud_list.empty()) ){
		std::list<WeatherCloud>::iterator it=cloud_list.begin();
		WeatherCloud *cloud;
		Sprite *spr_cloud;
		int number_to_render=0;
		int i=0;
		Point p;

		while (it != cloud_list.end()){
			cloud = &*it;
			if (cloud==NULL){
				it++;
				break;
			}

			p = cloud->getCurrentPoint();
			p = map_to_screen(p.x, p.y, mapr->cam.x, mapr->cam.y);

			number_to_render = cloud->getIntensity() + 1;

			if (cloud->getSize() == 0){
				spr_small_cloud->setClipW(600);
				spr_small_cloud->setClipX(cloud->getShape()*600);
				spr_cloud = spr_small_cloud;
			}
			if (cloud->getSize() == 1){
				spr_mid_cloud->setClipW(1200);
				spr_mid_cloud->setClipX(cloud->getShape()*1200);
				spr_cloud = spr_mid_cloud;
			}
			if (cloud->getSize() == 2){
				spr_big_cloud->setClipW(1800);
				spr_big_cloud->setClipX(cloud->getShape()*1800);
				spr_cloud = spr_big_cloud;
			}

			if (spr_cloud==NULL) break;
			spr_cloud->setDest(p);
			while (i < number_to_render){ // FIXME: might be quite bad for performance
				render_device->render(spr_cloud);
				i+=1;
			}
			i=0;
			it++;
        }
	}
    //if (cycle_i > time_of_rain) renderRainfall(); // FIXME: think about merging renderRainfall and rainfall()
    renderRainfall(); // TODO: remove this line, uncomment previous
}

ListWeatherCloud* ListWeatherCloud::getInstance() { return &ListWeatherCloud::instance; }

/*WeatherCloud* ListWeatherCloud::getFirstCloud(){
    // Note: only use this method if cloud_list.empty() is false
    return &cloud_list.front();
}*/

/*bool ListWeatherCloud::removeFirstCloud(){
    if (cloud_list.empty()) return false;
    cloud_list.pop_front();
    return true;
}*/

void ListWeatherCloud::createClouds(int cloudiness){
    WeatherCloud::SizeType size;
    WeatherCloud::IntensityType  intensity_type;
    int shape=0;
    std::list<WeatherCloud>::iterator it=cloud_list.begin();
    int i=0;
    Point p;

    while (cloudiness>4){ // create clouds
		if (i >= MAX_NUMBER_OF_CLOUDS-1) break;

		p = findValidPos();
		if (p.x == 1000) break; // no valid pos. for this cloud found

		size=WeatherCloud::SizeType(randBetween(0,2));
		if (cloudiness>24){ // intensity modest to thick depending on size
			if(size==WeatherCloud::SizeType(0)){
				intensity_type = WeatherCloud::IntensityType(1);
			}
			else intensity_type = WeatherCloud::IntensityType(2);
		}
		else{ // intensity thin to modest depending on size
			if(size==WeatherCloud::SizeType(0)){
				intensity_type = WeatherCloud::IntensityType(0);
			}
			else intensity_type = WeatherCloud::IntensityType(1);
		}
		shape=randBetween(0,5);
		/*std::cout<<"number: " << i << std::endl;
		std::cout<<"shape: " << shape << std::endl;
		std::cout<<"point x: " << p.x << "; y: " << p.y << std::endl;
		std::cout<<"intensity: " << intensity_type << std::endl;
		std::cout<<"size: " << size << std::endl;*/
		cloud_list.insert(it, WeatherCloud(p, size, intensity_type, shape));
		cloudiness-=(size+intensity_type+2)*3;
		i+=1;
		it++;
    }
}

Point ListWeatherCloud::findValidPos(uint d){
	bool is_valid_pos = false;
	int i=0, r0, r1;
	Point p;

	while(!is_valid_pos){
		if (i>9){
			logError("no valid position for this cloud!");
			return Point(1000,1000); // FIXME
		}
		r0 = randBetween(-d,d);
		r1 = randBetween(-d,d);

		is_valid_pos = mapr->collider.is_valid_position(mapr->cam.x + r0, mapr->cam.y + r1, MOVEMENT_FLYING, false);
		i+=1;

		// possible TODO: make sure two different clouds do not overlap
		// but this could be quite performance hungry...


	}
	p.x = mapr->cam.x + r0;
	p.y = mapr->cam.y + r1;
	return p;
}

void ListWeatherCloud::moveCloud(){

}

void ListWeatherCloud::clearUp(int frame_intervall){

	bool to_be_removed = false;
	std::list<WeatherCloud>::iterator it=cloud_list.begin();
	WeatherCloud *cloud;

    /*while (it != cloud_list.end()){
		*cloud = *it;
		if (cycle_i % frame_intervall == 0){
			to_be_removed = (!cloud->reduceIntensity());
		}

		if (to_be_removed){
			cloud_list.erase(it); // FIXME: there could be rare issues if single objects are erased
									// if there is a serious error with crash, it might be due to this
									// (I'm not sure about this)
			to_be_removed = false;
		}
		else it++;
    }*/
    state_is_initialized = false;

}
/*
void ListWeatherCloud::rainfall(){
    if (ListWeatherCloud::is_snow){
        snow();
    }
    else rain();
}

void ListWeatherCloud::snow(){

}

void ListWeatherCloud::rain(){

}*/

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
    int nr = 0;
    int i = 0;
    int j = 0;
    int r3 = 1; // changes type
    int r4 = 2;// changes offset
    int change_after = 10; // in frames
                                // Note: if there is ever a wind force implemented, it could be
                                // considered to depend this value upon it: more wind -> faster changes
    const int RADIUS = 22;
    int density = 3; // best values: 2, 3
    Rect screen_size = render_device->getContextSize();
    Point p;
    FPoint fp; // needed to check if is_valid_position

    if (is_strong_rainfall){
        density = 2;
        change_after = 6;
    }

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

    i = -RADIUS;
    j = -RADIUS;

    while(j < RADIUS){
        if (nr>47) nr=0;
        // update of position info should be rather slow...
          // the snowflakes appear to move with the character if true
        if ((cycle_i % (change_after * 10) == 0) || mapr->map_change) {
            flake_state[nr][1] = mapr->cam.x;
            flake_state[nr][2] = mapr->cam.y;
        }
        spr_flake->setClipW(8);
        spr_flake->setClipH(10);
        spr_flake->setClipX(flake_state[nr][0]*9);

        // TODO: take into account wind direction (variable 'direction')
        p = map_to_screen(flake_state[nr][1] + i, flake_state[nr][2] + j, mapr->cam.x, mapr->cam.y);
        if (cycle_i % change_after == 0) {
            flake_state[nr][4] = flake_state[nr][4] + randBetween(-1,1);
            flake_state[nr][5] = flake_state[nr][5] + randBetween(-1,2);
        }

        spr_flake->setOffset(flake_state[nr][3],0);
        spr_flake->setDestX(p.x + flake_state[nr][4]);
        spr_flake->setDestY(p.y + (flake_state[nr][5] % (screen_size.h/4)) - screen_size.h/4);

        if (mapr->collider.is_valid_position(flake_state[nr][1] + i, flake_state[nr][2] + j, MOVEMENT_FLYING, false)){
			// fade off effect for snowflakes near the ground
			if (flake_state[nr][5] % (screen_size.h/4) > screen_size.h/6){
				spr_flake->setClipY(10);
			}
			else spr_flake->setClipY(0);

			render_device->render(spr_flake);

        }
        i+=density;
        if (i>RADIUS){
            i= -2*RADIUS;
            j+=density;
        }
        nr+=1;
    }
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

    if (season_type==klima->WINTER) is_snow=true;
    // Note: Likeliness of rain and clouds is not ONLY influenced by the
        // WeatherClimate setting, but is partly also random
        // the random part of both cloudiness and cycle_max is done in
        // ListWeatherClouds
    cloudiness = 10;
    // TODO: wind variables changes
      // ...
    if (humidity_type==WeatherClimate::NORMAL){
        cloudiness+=16;
    }
    else if(humidity_type==WeatherClimate::WET){
        cloudiness+=32;
    } // else DRY, stays
    if (season_type==WeatherClimate::SUMMER){ // dryer in the summer
        cloudiness-=10;
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
