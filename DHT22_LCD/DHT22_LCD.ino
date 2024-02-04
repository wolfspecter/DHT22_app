#include <Wire.h>
#include "rgb_lcd.h"
#include "DHT.h"
#include <WiFi.h>
#include "time.h"
#include <HTTPClient.h>
#include "pitches.h"

#define DHT22_PIN 12
#define BUZZ_PIN 15

#define ssid "Tuan Dat"
#define password "tuandat2002"

String GOOGLE_SCRIPT_ID = "AKfycbyPB0AF9Rj3p9UwhNwhwzN9jTCXUlMQytnVkFzt276UJTxdnWahGmuFxrUNR3VeUPS1FQ";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 0;

DHT dht22(DHT22_PIN, DHT22);

rgb_lcd lcd;
const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

byte flame[8]={
  0b00010,
  0b00110,
  0b01111,
  0b01110,
  0b11101,
  0b11111,
  0b11011,
  0b01010
};

byte snow[8]={
  0b00000,
  0b00000,
  0b10101,
  0b01110,
  0b11011,
  0b01110,
  0b10101,
  0b00000
};

float temp(){
  float tempC=dht22.readTemperature();
  return tempC;
}

int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

void setup(){
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());

  dht22.begin();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  lcd.begin(16,2);
  lcd.setRGB(colorR,colorG,colorB);
  #if 1
    lcd.createChar(0,flame);
    lcd.createChar(1,snow);
  #endif

    delay(1000);
    for (int thisNote = 0; thisNote < 8; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(BUZZ_PIN, melody[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(BUZZ_PIN);
    }
}

void loop(){

  lcd.setCursor(0,0);
  lcd.print("nhiet do: ");
  lcd.print(temp());
  lcd.write((unsigned char)1);
  
  if(temp()<=2||temp()>=6){
    for (int thisNote = 0; thisNote < 8; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(BUZZ_PIN, melody[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(BUZZ_PIN);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    static bool flag = false;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }
    char timeStringBuff[50]; //50 chars should be enough
    strftime(timeStringBuff, sizeof(timeStringBuff), "%d", &timeinfo);
    String dasString(timeStringBuff);

    strftime(timeStringBuff, sizeof(timeStringBuff), "%B", &timeinfo);
    String masString(timeStringBuff);

    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y", &timeinfo);
    String yasString(timeStringBuff);

    strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", &timeinfo);
    String tasString2(timeStringBuff);

    strftime(timeStringBuff, sizeof(timeStringBuff), "%H%M%S", &timeinfo);
    String tasString1(timeStringBuff);
    
    Serial.print("Time:");
    Serial.println(tasString1);
    Serial.println("day:");
    Serial.println(dasString);
    String urlFinal = "https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+"date=" + tasString1 + "&sensor=" + String(temp()) + "&day=" + dasString + "&month=" + masString + "&year=" + yasString +"&time=" +tasString2;
    Serial.print("POST data to spreadsheet:");
    Serial.print("POST data to spreadsheet:");
    Serial.println(urlFinal);
    HTTPClient http;
    http.begin(urlFinal.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET(); 
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    //---------------------------------------------------------------------
    //getting response from google sheet
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);    
    }
    //---------------------------------------------------------------------
    http.end();
  }
  delay(1000);
  }