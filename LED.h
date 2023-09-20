#include "WString.h"
#ifndef LED_H
#define LED_H

#define Thunderstorm 0
#define Drizzle 1
#define Rain 2
#define Snow 3
#define Clear_sky 4
#define Clouds 5
#define Mist 6
#define Other_weather 7

#include <LiquidCrystal_I2C.h>

class LED{
  public:
    LED(int red, int green, int blue, int lcdAddress, int lcdColumns, int lcdRows); // redPin, greenPin, bluePin.
    void thunderstorm(); 
    void drizzle(int r, int g, int b);
    void rain(int r, int g, int b);
    void snow();
    void clear_sky(int r, int g, int b);
    void clouds(int r, int g, int b);
    void mist(int r, int g, int b);
    void other_weather(int r, int g, int b);
    void off();
    void run();
    void set_moodlight(bool now_state, void (*dht_get)(), String (*weather_colors), int id, String description, float temp, float humi);

    void initLCD();
    void printLCD(int c, int r, String text);
    void printLCD(int c, int r, char* text);
    void printLCD(int c, int r, float value);
    void printLCD(int c, int r, double value);
    void clearLCD();
    
  private:
    int red, green, blue;
    int mode;
    
    int onMil;
    int offMil;
    
    int snowVal;
    int thunderVal;
    
    bool blinkOn;
    unsigned long nextMil;

    LiquidCrystal_I2C lcd;
};

#endif
