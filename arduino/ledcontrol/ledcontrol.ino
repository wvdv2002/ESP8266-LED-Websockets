#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <Adafruit_NeoPixel.h>

#include <Hash.h>
#include <EEPROM.h>
#include <WebSockets.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include "SettingsServer.h"


#define USE_SERIAL Serial
// Wifi credentials

extern "C" {
  #include "user_interface.h"
}

// Defining LED strip
#define NUM_LEDS 240                 //Number of LEDs in your strip
#define DATA_PIN 15                //Using WS2812B -- if you use APA102 or other 4-wire LEDs you need to also add a clock pin
#define DATAFASTLED_PIN 0
#define BRIGHTNESS 255
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRBW + NEO_KHZ800);

CRGB leds[NUM_LEDS];
CRGBSet ledSet(leds, NUM_LEDS);    //Trying LEDSet from FastLED
void putOnStrip(void);
//Some Variables
byte myEffect = 1;                  //what animation/effect should be displayed

byte myHue = 33;                    //I am using HSV, the initial settings display something like "warm white" color at the first start
byte mySaturation = 168;
byte myValue = 255;
unsigned int myWhiteLedValue=0;
byte rainbowHue = myHue;            //Using this so the rainbow effect doesn't overwrite the hue set on the website

int flickerTime = random(200, 400);
int flickerLed;
int flickerValue = 110 + random(-3, +3); //70 works nice, too
int flickerHue = 33;

bool eepromCommitted = true;      

unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long lastChangeTime = 0;
unsigned long currentChangeTime = 0;
unsigned long startTimeSleepTimer = 0;
unsigned long sleepTime = 0;
bool inSleep = 0;

#include "LEDanimations.h"
#include "LEDWebsockets.h"
int oldPWMValue = 9999;

void writeWhiteLedPWMIfChanged(int value)
{

  if (oldPWMValue != value)
  {
    oldPWMValue = value;
    putOnStrip();
  }
}

void putOnStrip(void){
  uint8_t pwmTemp=oldPWMValue/4;
  for(int i=0;i<strip.numPixels();i++)
  {
    strip.setPixelColor(i,leds[i].r,leds[i].g,leds[i].b,pwmTemp);  
  }
  strip.show();
}


void setup() {
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  analogWriteFreq(200);
  writeWhiteLedPWMIfChanged(0);  
  writeWhiteLedPWMIfChanged(1);  
  EEPROM.begin(6);  // Using simulated EEPROM on the ESP8266 flash to remember settings after restarting the ESP
  Serial.begin(115200);
  Serial.println("Ledtest example");
  LEDS.addLeds<WS2812B,DATAFASTLED_PIN,GRB>(leds,NUM_LEDS);  // Initialize the LEDs


  // Reading EEPROM
  myEffect = 1;                         // Only read EEPROM for the myEffect variable after you're sure the animation you are testing won't break OTA updates, make your ESP restart etc. or you'll need to use the USB interface to update the module.
  
//  myEffect = EEPROM.read(0); //blocking effects had a bad effect on the website hosting, without commenting this away even restarting would not help
  myHue = EEPROM.read(1);
  mySaturation = EEPROM.read(2);
  myValue = EEPROM.read(3);

//  LEDS.showColor(CHSV(0,255,255));
  ledSet = CHSV(0,100,100);
  putOnStrip();
  delay(100);                                         //Delay needed, otherwise showcolor doesn't light up all leds or they produce errors after turning on the power supply - you will need to experiment
  ledSet = CHSV(0,100,100);

  putOnStrip();
 setupWiFi();
 //  WiFi.begin("EAZHuis", "HommeJan54");
  ledSet = CHSV(myHue, mySaturation, myValue);
  putOnStrip();
  startSettingsServer();
  //Websocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}


void startSleepTimer(int value){
  sleepTime = value * 1000;
  startTimeSleepTimer = millis();
  inSleep = 1;
}

int getSleepTimerRemainingTime(void){
  return (sleepTime-startTimeSleepTimer)/1000/60;
}

void disableSleepTimer(void){
  inSleep = 0;
}

void whiteFadeToBlackBy(int amount){
  if(oldPWMValue>amount){
    oldPWMValue-= amount;
  }
  else if(oldPWMValue!=0){
    oldPWMValue=0;
  }
}

void loop() {
  webSocket.loop();                           // handles websockets
  settingsServerTask();
  if (myEffect!=5)
  {
    writeWhiteLedPWMIfChanged(myWhiteLedValue);  
  }

  if(inSleep == 1){
    if (millis()-startTimeSleepTimer>sleepTime){
      myEffect = 5;
      inSleep = 0;
   }
  }
  

  EVERY_N_MILLISECONDS( 20 ) {
        ledSet = CHSV(myHue, mySaturation, myValue);
        putOnStrip();
        //LEDS.show();
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
      EVERY_N_MILLISECONDS( 50 ) {
      whiteFadeToBlackBy(8);
      ledSet.fadeToBlackBy(2);
      putOnStrip();
      //LEDS.show();
      
      }
      break;
    case 6: // loop through hues with all leds the same color. Can easily be changed to display a classic rainbow loop
      EVERY_N_MILLISECONDS( 250 ) {
      rainbowHue = rainbowHue + 1;
      ledSet = CHSV(rainbowHue, mySaturation, myValue);
      putOnStrip();
     }
      break;
    case 7: // make a single, random LED act as a candle
      currentTime = millis();
      ledSet.fadeToBlackBy(1);
      leds[flickerLed] = CHSV(flickerHue, 255, flickerValue);
      flickerTime = random(150, 500);
      if (currentTime - previousTime > flickerTime) {
        flickerValue = 110 + random(-10, +10); //70 works best
        flickerHue = 33; //random(33, 34);
        previousTime = currentTime;
        putOnStrip();
        //LEDS.show();
      }
      break;
    default: 
//      LEDS.showColor(CRGB(0, 255, 0)); // Bright green in case of an error
      ledSet = CRGB(0,255,0);
      putOnStrip();
    break;
  }

  
  // EEPROM-commit and websocket broadcast -- they get called once if there has been a change 1 second ago and no further change since. This happens for performance reasons.
  currentChangeTime = millis();
  if (currentChangeTime - lastChangeTime > 5000 && eepromCommitted == false) {
     Serial.print("Heap free: ");  
     Serial.println(system_get_free_heap_size());

    EEPROM.commit();
    eepromCommitted = true;
    String websocketStatusMessage = "H" + String(myHue) + ",S" + String(mySaturation) + ",V" + String(myValue) + ",W" + String(myWhiteLedValue) + ",F" + String(getSleepTimerRemainingTime()); //Sends a string with the HSV and white led  values to the client website when the conection gets established
    webSocket.broadcastTXT(websocketStatusMessage); // Tell all connected clients which HSV values are running
    //LEDS.showColor(CRGB(0, 255, 0));  //for debugging to see when if-clause fires
    //delay(50);                        //for debugging to see when if-clause fires
  }
}
