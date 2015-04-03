//==============================================================================

#ifndef __WeatherClimate_H__
	#define __WeatherClimate_H__

// Note for me: it might make sense, to use the existing class Entity as base

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
