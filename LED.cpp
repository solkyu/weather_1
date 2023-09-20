#include "WString.h"
#include <Arduino.h>
#include "LED.h"
#include <LiquidCrystal_I2C.h>

LED::LED(int Ired, int Igreen, int Iblue, int lcdAddress, int lcdColumns, int lcdRows) : lcd(lcdAddress, lcdColumns, lcdRows){
  red = Ired;
  green = Igreen;
  blue = Iblue;
  
  mode = Clear_sky;
  
  pinMode(red ,OUTPUT);
  pinMode(green ,OUTPUT);
  pinMode(blue ,OUTPUT);
}

void LED::thunderstorm(){
  mode = Thunderstorm;
  blinkOn = true;

  onMil = 500;
  offMil = 500;
  thunderVal = 0;
  analogWrite(red, 255);
  analogWrite(green, 255);
  analogWrite(blue, 255);

  nextMil = millis() + onMil;
}

void LED::snow(){
  mode = Snow;

  onMil = 30;
  offMil = 400;
  snowVal = 0;
  analogWrite(red, 255);
  analogWrite(green, 255);
  analogWrite(blue, 255);
  
  blinkOn = true;
  
  nextMil = millis() + onMil;
}

void LED::drizzle(int r, int g, int b){
  mode = Drizzle;

  analogWrite(red, r);
  analogWrite(green, g);
  analogWrite(blue, b);
}

void LED::rain(int r, int g, int b){
  mode = Rain;

  analogWrite(red, r);
  analogWrite(green, g);
  analogWrite(blue, b);
}

void LED::clear_sky(int r, int g, int b){
  mode = Clear_sky;
  
  analogWrite(red, r);
  analogWrite(green, g);
  analogWrite(blue, b);
}

void LED::clouds(int r, int g, int b){
  mode = Clouds;
  
  analogWrite(red, r);
  analogWrite(green, g);
  analogWrite(blue, b);
}

void LED::mist(int r, int g, int b){
  mode = Mist;
  
  analogWrite(red, r);
  analogWrite(green, g);
  analogWrite(blue, b);
}

void LED::other_weather(int r, int g, int b){
  mode = Other_weather;
    
  analogWrite(red, r);
  analogWrite(green, g);
  analogWrite(blue, b);
}

void LED::off(){

  analogWrite(red, 0);
  analogWrite(green, 0);
  analogWrite(blue, 0);
}

void LED::run(){
  if (mode == Thunderstorm){
    if (thunderVal >= 2){
      thunderVal = 0;
      nextMil = millis() + 1000;
    }
    else if (blinkOn && millis() > nextMil){
      analogWrite(red, 0);
      analogWrite(green, 0);
      analogWrite(blue, 0);
      
      blinkOn = false;
      nextMil = millis() + offMil;
    }
    else if (!blinkOn && millis() > nextMil){
      analogWrite(red, 255);
      analogWrite(green, 255);
      analogWrite(blue, 255);
      
      blinkOn = true;
      nextMil = millis() + onMil;

      thunderVal++;
    }
  }
  if (mode == Snow){
    if (blinkOn){
      analogWrite(red, snowVal);
      analogWrite(green, snowVal);
      analogWrite(blue, snowVal);
      if (millis() > nextMil){
        nextMil = millis() + onMil;
        snowVal++;
      }
      if (snowVal >= 255){
        blinkOn = false;
        nextMil = millis() + offMil;
      }
    }
    else if (!blinkOn){
      analogWrite(red, snowVal);
      analogWrite(green, snowVal);
      analogWrite(blue, snowVal);
      if (millis() > nextMil){
        nextMil = millis() + onMil;
        snowVal--;
      }
      if (snowVal <= 0){
        blinkOn = true;
        nextMil = millis() + offMil;
      }
    }
  }
}

void LED::initLCD(){
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void LED::printLCD(int c, int r, String text){
  lcd.setCursor(c, r);
  lcd.print(text);
}
void LED::printLCD(int c, int r, float value) {
    lcd.setCursor(c, r);
    lcd.print(value);
}
void LED::printLCD(int c, int r, char* text) {
    lcd.setCursor(c, r);
    lcd.print(text);
}
void LED::printLCD(int c, int r, double value) {
    lcd.setCursor(c, r);
    lcd.print(value);
}

void LED::clearLCD(){
  lcd.clear();
}
void LED::set_moodlight(bool now_state, void (*dht_get)(), String (*weather_colors), int id, String description, float temp, float humi){
  if (now_state == true){
    dht_get();
  }
  if (now_state == false){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(description);
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.setCursor(2, 1);
    lcd.print(temp);
    lcd.setCursor(6, 1);
    lcd.print("C");
    lcd.setCursor(8, 1);
    lcd.print("H:");
    lcd.setCursor(10, 1);
    lcd.print(humi);
    lcd.setCursor(14, 1);
    lcd.print("%");
  }
  
  if (id >= 200 && id <= 232){
    thunderstorm();
  }
  else if (id >= 300 && id <= 321){
    drizzle(atoi(weather_colors[0].c_str()), atoi(weather_colors[1].c_str()), atoi(weather_colors[2].c_str()));
  }
  else if (id >= 500 && id <= 531){
    rain(atoi(weather_colors[3].c_str()), atoi(weather_colors[4].c_str()), atoi(weather_colors[5].c_str()));
  }
  else if (id >= 600 && id <= 622){
    snow();
  }
  else if (id == 701){
    mist(atoi(weather_colors[12].c_str()),atoi(weather_colors[13].c_str()), atoi(weather_colors[14].c_str()));
  }
  else if (id == 800){
    clear_sky(atoi(weather_colors[6].c_str()), atoi(weather_colors[7].c_str()), atoi(weather_colors[8].c_str()));
  }
  else if (id >= 801 && id <= 804){
    clouds(atoi(weather_colors[9].c_str()), atoi(weather_colors[10].c_str()), atoi(weather_colors[11].c_str()));

  }
  else{
    other_weather(atoi(weather_colors[15].c_str()), atoi(weather_colors[16].c_str()), atoi(weather_colors[17].c_str()));
  }
}