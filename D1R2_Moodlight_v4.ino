#include <Arduino.h>

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>
#include <WiFiClient.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Hash.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#include "LED.h"
#include "index_html.h"
#include "mycrypto.h"

ESP8266WiFiMulti WiFiMulti;

LED led0(D4, D3, D5, 0x27, 16, 2);
DHT_Unified dht(D6, DHT22);
char key[] = "thisisprivateKEYmynameiskyu0409ThisIsMyLittleCloud.krsince09";
MyCrypto mycrypto(key);

const int reset = D0;

static unsigned long next;
static unsigned long btn_press;
const unsigned long reset_time = 5000;

const int SP_pass = 0;

const char* ssid     = "MK1";
const char* password = "";

static bool now_state = false;

const int debounceDelay = 50;
const unsigned long longPressDuration = 5000;

unsigned long lastDebounceTime = 0;
bool lastButtonState = LOW;
bool buttonPressed = false;
unsigned long buttonPressedStartTime = 0;

String weather_colors[18];
String global_json_data = "";
String global_method = "";
String golbal_API = "";

AsyncWebServer server(80); 

char state;

const char* weather_param_names[] = {
  "drizzle_r", "drizzle_g", "drizzle_b",
  "rain_r", "rain_g", "rain_b",
  "clearSky_r", "clearSky_g", "clearSky_b",
  "clouds_r", "clouds_g", "clouds_b",
  "mist_r", "mist_g", "mist_b",
  "otherWeather_r", "otherWeather_g", "otherWeather_b"
};

int read_pass_ssid_pw(int address);
void write_pass_ssid_pw(int address, char state);
String readStringFromEEPROM(int address);
void writeStringToEEPROM(int address, String data);
void JsonParser(String payload);
void lcd_moodlight(int id, String description, float temp, float feels_like);
void reset_board();
void dht_get();
String get_data();

void setup(){
  Serial.begin(115200);
  EEPROM.begin(4096);
  pinMode(reset, INPUT);

  dht.begin();

  led0.initLCD();

  state = read_pass_ssid_pw(SP_pass);

  if(state == '{'){
    led0.printLCD(0, 0, "2021.12.25");
    led0.printLCD(0, 1, "Made by Kyu");
    delay(1000);

    led0.clearLCD();
    led0.printLCD(0, 0, "Reading");
    led0.printLCD(0, 1, "System Data");

    String json_data = readStringFromEEPROM(0);

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, json_data);

    String method = doc["method"];
    global_method = method;
    String WiFi_ID = doc["WiFi_ID"];
    String WiFi_PW = doc["WiFi_PW"];

    if (strcmp(global_method.c_str(), "OpenWeatherMap") == 0){
      String API = doc["API"];
      golbal_API = API;
      for(int i = 0; i < sizeof(weather_param_names) / sizeof(weather_param_names[0]); i++){
        String s = doc["color"][i];
        weather_colors[i] = s;
      }
    }

    global_json_data = json_data;
    
    led0.clearLCD();
    led0.printLCD(0, 0, "Connecting to");
    led0.printLCD(0, 1, "Wi-Fi");
    
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(WiFi_ID.c_str(), WiFi_PW.c_str());
  }
  else{
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();

    led0.printLCD(0, 0, "WiFi_ID:");
    led0.printLCD(8, 0, "MK1");
    led0.printLCD(0, 1, IP.toString());

    //Serial.println(IP);
    // Print ESP8266 Local IP Address
    //Serial.println(WiFi.localIP());

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", reinterpret_cast<const uint8_t*>(weather_api_html), weather_api_html_len);
    });

    server.on("/MyLittleCloud", HTTP_POST, [](AsyncWebServerRequest *request){
      if(request->hasParam("ID", true)){
        DynamicJsonDocument jsonDoc(1024);

        jsonDoc["method"] = "MyLittleCloud";
        jsonDoc["WiFi_ID"] = request->getParam("ID", true)->value();
        jsonDoc["WiFi_PW"] = request->getParam("PW", true)->value();
        jsonDoc["MLC_ID"] = request->getParam("M_ID", true)->value();
        jsonDoc["MLC_PW"] = request->getParam("M_PW", true)->value();

        String jsonString = "";
        
        serializeJson(jsonDoc, jsonString);
        //Serial.println(jsonString);
        writeStringToEEPROM(0, jsonString);
        request->send(200, "text/html", "Data received");
        server.end();

        ESP.restart();
      } else {
        request->send(400, "text/html", "An error has occurred. please try again"); // Bad Request 응답
      }
    });

    server.on("/OpenWeatherMap", HTTP_POST, [](AsyncWebServerRequest *request){
      if(request->hasParam("ID", true) && request->hasParam("API", true)){
        DynamicJsonDocument jsonDoc(2048);

        jsonDoc["method"] = "OpenWeatherMap";
        jsonDoc["WiFi_ID"] = request->getParam("ID", true)->value();
        jsonDoc["WiFi_PW"] = request->getParam("PW", true)->value();
        jsonDoc["API"] = request->getParam("API", true)->value();
        for (int i = 0; i < sizeof(weather_param_names) / sizeof(weather_param_names[0]); i++) {
          jsonDoc["color"][i] = request->getParam(weather_param_names[i], true)->value();
        }
        String jsonString = "";
        serializeJson(jsonDoc, jsonString);
        
        writeStringToEEPROM(0, jsonString);

        request->send(200, "text/html", "Data received");
        server.end();

        ESP.restart();
      } else {
        request->send(400, "text/html", "An error has occurred. please try again"); // Bad Request 응답
      }
    });
    // Start server
    server.begin();
  }  
}
 
void loop(){
  if (strcmp(global_method.c_str(), "OpenWeatherMap") == 0 && digitalRead(reset) == LOW){
    btn_press = millis();
    if (millis() > next && (WiFiMulti.run() == WL_CONNECTED)) {
      JsonParser(get_data(golbal_API));
      next = millis() + 20000;
    }
  }
  if (strcmp(global_method.c_str(), "MyLittleCloud") == 0 && digitalRead(reset) == LOW){
    btn_press = millis();
    if (millis() > next && (WiFiMulti.run() == WL_CONNECTED)) {
      DynamicJsonDocument doc(4096);
      deserializeJson(doc, global_json_data);
      String MLC_ID = doc["MLC_ID"];
      String MLC_PW = doc["MLC_PW"];

      String data = MLC_ID + "/" + MLC_PW;
      
      data = mycrypto.encrypt(data);
      
      //String MLC_URL = "http://www.mylittlecloud.kr/user/get_data/" + data;
      String MLC_URL = "http://192.168.55.204/user/get_data/" + data;
      String json_data = get_data(MLC_URL);
      //Serial.println(json_data);
      deserializeJson(doc, json_data);

      for(int i = 0; i < sizeof(weather_param_names) / sizeof(weather_param_names[0]); i++){
        String s = doc["color"][i];
        weather_colors[i] = s;
      }
      String message = doc["message"];
      String token = doc["token"];

      MLC_URL = "";
      //MLC_URL = "http://www.mylittlecloud.kr/weather/city_weather/" + token;
      MLC_URL = "http://192.168.55.204/weather/city_weather/" + token;

      JsonParser(get_data(MLC_URL));

      next = millis() + 20000;
    }
  }

  int buttonState = digitalRead(reset);

  if (buttonState != lastButtonState) {
    if (millis() - lastDebounceTime >= debounceDelay) {
      // 버튼이 눌린 경우
      if (buttonState == HIGH) {
        now_state = !now_state;
        next = millis();
        //Serial.println("button pressed");
        buttonPressed = true; // 버튼이 눌렸음
        buttonPressedStartTime = millis(); // 버튼이 눌린 시작 시간 기록
      }
      else {
        // 버튼이 떨어진 경우
        if (buttonPressed) {
          // 버튼이 눌린 상태였다면 버튼이 떨어진 시점 확인
          unsigned long buttonReleaseTime = millis();
          if (buttonReleaseTime - buttonPressedStartTime >= longPressDuration) {
            reset_board();
          }
          buttonPressed = false;
        }
      }
    }

    lastDebounceTime = millis();
  }

  lastButtonState = buttonState;

  led0.run();
}

void JsonParser(String payload) {
  DynamicJsonDocument doc(payload.length()+3);  //데이터 크기에 따라 크기 조정
  deserializeJson(doc, payload);

  int id = doc["weather"][0]["id"];
  String description = doc["weather"][0]["description"];
  float temp = doc["main"]["temp"].as<float>() - 273;
  float humidity = doc["main"]["humidity"].as<float>();

  led0.set_moodlight(now_state, dht_get, weather_colors, id, description, temp, humidity);
}

void writeStringToEEPROM(int address, String data) {
  for (int i = 0; i < data.length(); ++i) {
    EEPROM.write(address + i, data[i]);
  }
  EEPROM.write(address + data.length(), '\0');  // 문자열 종료 문자(null) 저장
  EEPROM.commit();
}

String readStringFromEEPROM(int address) {
  String data = "";
  char character;
  int i = 0;
  while ((character = EEPROM.read(address + i)) != '\0') {
    data += character;
    ++i;
  }
  return data;
}

void write_pass_ssid_pw(int address, char state){
  EEPROM.write(address, state);
  EEPROM.commit();
}

int read_pass_ssid_pw(int address){
  char c;
  c = EEPROM.read(address);

  return c;
}

void reset_board(){
  write_pass_ssid_pw(SP_pass, 'x');
  led0.off();
  ESP.restart();
}

void dht_get(){
  led0.clearLCD();
  led0.printLCD(0, 0, "Indoor Condition");
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    led0.printLCD(0, 1, "tempERR");
  }
  else {
    led0.printLCD(0, 1, "T:");
    led0.printLCD(2, 1, event.temperature);
    led0.printLCD(6, 1, "C");
  }
  
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    led0.printLCD(8, 1, "humiERR");
  }
  else {
    led0.printLCD(8, 1, "H:");
    led0.printLCD(10, 1, event.relative_humidity);
    led0.printLCD(14, 1, "%");
  }
}

String get_data(String url){
  WiFiClient client;
  HTTPClient http;
  if (http.begin(client, url.c_str())) {  // HTTP
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        return payload;
      }
    } else {
      //Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      led0.clearLCD();
      led0.printLCD(0, 0, "E:[HTTP] failed");
      led0.printLCD(0, 1, "E:00001");
      return "ERR1";
    }
    http.end();
  } else {
    //Serial.printf("[HTTP} Unable to connect\n");
    led0.clearLCD();
    led0.printLCD(0, 0, "E:[HTTP] failed");
    led0.printLCD(0, 1, "E:00002");
    return "ERR2";
  }
  return "ERR3";
}
