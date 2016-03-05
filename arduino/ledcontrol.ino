#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <EEPROM.h>

// Wifi credentials
const char* ssid = "your SSID";
const char* password = "your password";


// Defining LED strip
#define NUM_LEDS 30                 //Number of LEDs in your strip
#define DATA_PIN 12                 //Using WS2812B -- if you use APA102 or other 4-wire LEDs you need to also add a clock pin
CRGB leds[NUM_LEDS];

//Some Variables
byte myEffect = 1;                  //what animation/effect should be displayed

byte myHue = 33;                    //I am using HSV, the initial settings display something like "warm white" color at the first start
byte mySaturation = 168;
byte myValue = 255;

byte rainbowHue = myHue;            //Using this so the rainbow effect doesn't overwrite the hue set on the website

int flickerTime = random(200, 400);
int flickerLed;
int flickerValue = 110 + random(-3, +3); //70 works nice, too
int flickerHue = 33;

bool eepromCommitted = false;

unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long lastChangeTime = 0;
unsigned long currentChangeTime = 0;

#include "LEDanimations.h"
#include "LEDWebsockets.h"

void setup() {
  EEPROM.begin(4);                                    // Using simulated EEPROM on the ESP8266 flash to remember settings after restarting the ESP
  LEDS.addLeds<WS2812B,DATA_PIN,GRB>(leds,NUM_LEDS);  // Initialize the LEDs

  // Reading EEPROM
  myEffect = EEPROM.read(0);                          // Only use this after you're sure the animation you are testing won't break OTA updates, make your ESP restart etc. or you'll need to use the USB interface to update the module.
  myHue = EEPROM.read(1);
  mySaturation = EEPROM.read(2);
  myValue = EEPROM.read(3);
  
  delay(100);                                         //Delay needed, otherwise showcolor doesn't light up all leds or they produce errors after turning on the power supply - you will need to experiment
  LEDS.showColor(CHSV(myHue, mySaturation, myValue));

// Starting Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    delay(1000);
  }

// All this below is for OTA-updates
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("myESP");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.begin();
  
  //Websocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}


void loop() {
  ArduinoOTA.handle();                        // handles OTA-updates
  webSocket.loop();                           // handles websockets
  
switch (myEffect) {                           // switches between animations
    case 1: // Solid Color
      currentTime = millis();
      if (currentTime - previousTime > 20) {
        for (int i = 0 ; i < NUM_LEDS; i++ ) {
        leds[i] = CHSV(myHue, mySaturation, myValue);
        }
        previousTime = currentTime;
        LEDS.show();
      }
      break;
    case 2: // Ripple effect
      ripple();
      break;
    case 3: // Cylon effect
      cylon();
      break;
    case 4: // Fire effect
      Fire2012();
      break;
    case 5: // Turn off all LEDs
      for (int i = 0 ; i < NUM_LEDS; i++ ) {
        if( leds[i] ) {
        LEDS.showColor(CHSV(0, 0, 0));
        } 
      }     
      break;
    case 6: // loop through hues with all leds the same color. Can easily be changed to display a classic rainbow loop
      currentTime = millis();
      if (currentTime - previousTime > 250) {
        rainbowHue = rainbowHue + 1;
        for (int i = 0 ; i < NUM_LEDS; i++ ) {
        leds[i] = CHSV(rainbowHue, mySaturation, myValue);
        } 
      previousTime = currentTime;
      LEDS.show();
      }
      
      break;
    case 7: // make a single, random LED act as a candle
      currentTime = millis();
      for (int i = 0 ; i < NUM_LEDS; i++ ) {
          leds[i].fadeToBlackBy(2);
      } 
      leds[flickerLed] = CHSV(flickerHue, 255, flickerValue);
      flickerTime = random(150, 500);
      if (currentTime - previousTime > flickerTime) {
        flickerValue = 110 + random(-10, +10); //70 works best
        flickerHue = 33; //random(33, 34);
        previousTime = currentTime;
        LEDS.show();
      }
      break;
    default: 
      LEDS.showColor(CRGB(0, 255, 0)); // Bright green in case of an error
    break;
  }
  
  // EEPROM-commit and websocket broadcast -- they get called once if there has been a change 1 second ago and no further change since. This happens for performance reasons.
  currentChangeTime = millis();
  if (currentChangeTime - lastChangeTime > 1000) {
    if (eepromCommitted == false) {
    EEPROM.commit();
    eepromCommitted = true;
    String websocketStatusMessage = "H" + String(myHue) + ",S" + String(mySaturation) + ",V" + String(myValue);
    webSocket.broadcastTXT(websocketStatusMessage); // Tell all connected clients which HSV values are running
    //LEDS.showColor(CRGB(0, 255, 0));  //for debugging to see when an if-clause fires without having to use a serial monitor - a handy little flash of green light
    //delay(50);                        //for debugging to see when if-clause fires
    }
  }
}
