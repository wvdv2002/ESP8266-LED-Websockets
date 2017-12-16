#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <Adafruit_NeoPixel.h>
#include <PubSubClient.h>

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
unsigned int myAnimationSpeed = 50;
unsigned int myWhiteLedValue=0;
byte rainbowHue = myHue;            //Using this so the rainbow effect doesn't overwrite the hue set on the website

void changeHue(int);
void changeSaturation(int);
void changeRGBIntensity(int);
void changeWhiteIntensity(int);
void changeLedAnimation(int);
void changeAnimationSpeed(int);

bool eepromCommitted = true;      
unsigned long lastChangeTime = 0;
unsigned long currentChangeTime = 0;
unsigned long startTimeSleepTimer = 0;
unsigned long sleepTime = 0;
bool inSleep = 0;

#include "LEDanimations.h"
#include "LEDWebsockets.h"
int oldPWMValue = 9999;

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

  // Reading EEPROM
  myEffect = 1;                         // Only read EEPROM for the myEffect variable after you're sure the animation you are testing won't break OTA updates, make your ESP restart etc. or you'll need to use the USB interface to update the module.
//  myEffect = EEPROM.read(0); //blocking effects had a bad effect on the website hosting, without commenting this away even restarting would not help
  myHue = EEPROM.read(1);
  mySaturation = EEPROM.read(2);
  myValue = EEPROM.read(3);

  ledSet = CHSV(0,100,100);
  putOnStrip();
  delay(100);                                         
  ledSet = CHSV(0,100,100);
  putOnStrip();
  setupWiFi();
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

int getAnimationSpeed(void) {
  return myAnimationSpeed;
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
  if (myEffect!=0)
  {
    writeWhiteLedPWMIfChanged(myWhiteLedValue);  
  }
  if(inSleep == 1){
    if (millis()-startTimeSleepTimer>sleepTime){
      myEffect = 0;
      inSleep = 0;
   }
  }
  
  EVERY_N_MILLISECONDS( myAnimationSpeed ) {
   switch(myEffect){ //Override some led animation code here if necessary.
      case 0: // Turn off all LEDs
        whiteFadeToBlackBy(8);
        ledSet.fadeToBlackBy(2);
        break;
      case 6: // loop through hues with all leds the same color. Can easily be changed to display a classic rainbow loop
        rainbowHue = rainbowHue + 1;
        ledSet = CHSV(rainbowHue, mySaturation, myValue);
        break;
      default: 
        if(myEffect<AMOUNT_OF_ANIMATIONS){
          ledPatterns[myEffect]();
        }else{  //Generate  visual error if too high a number of animation is set.
          ledSet = CRGB(0,255,0);
        }
      break;
    }
    putOnStrip();
  }

  
  // EEPROM-commit and websocket broadcast -- they get called once if there has been a change 1 second ago and no further change since. This happens for performance reasons.
  currentChangeTime = millis();
  if (currentChangeTime - lastChangeTime > 5000 && eepromCommitted == false) {
     Serial.print("Heap free: ");  
     Serial.println(system_get_free_heap_size());

    EEPROM.commit();
    eepromCommitted = true;
    String aMessage = getStatusString();
    webSocket.broadcastTXT(aMessage); // Tell all connected clients which HSV values are running
  }
}


void writeWhiteLedPWMIfChanged(int value)
{
  if (oldPWMValue != value)
  {
    oldPWMValue = value;
    putOnStrip();
  }
}


void changeLedAnimation(int animation){
  flickerLed = random(0,NUM_LEDS-1); //Update flickerled position, hacky I know ;(
  if (myEffect != animation) {  // only do stuff when there was a change
    myEffect = animation;
    rainbowHue = myHue;
    EEPROM.write(0, myEffect);       //stores the variable but needs to be committed to EEPROM before being saved - this happens in the loop
    lastChangeTime = millis();
    eepromCommitted = false;
  }
}

void changeHue(int hue){
  if (myHue != hue) {
    myHue = hue;
    rainbowHue = myHue;
    EEPROM.write(1, myHue);
    lastChangeTime = millis();
    eepromCommitted = false;
  }
}

void changeSaturation(int saturation){
  if (mySaturation != saturation) {
    mySaturation = saturation;
    EEPROM.write(2, mySaturation);
    lastChangeTime = millis();
    eepromCommitted = false;
  }
}

void changeWhiteIntensity(int value){
  if (myWhiteLedValue != value) {
    myWhiteLedValue = value;
    EEPROM.write(4, myWhiteLedValue&&255);
    EEPROM.write(5, myWhiteLedValue/256);
    lastChangeTime = millis();
    eepromCommitted = false;
  }
}

void changeRGBIntensity(int value){
  if (myValue != value) {
    myValue = value;
    EEPROM.write(3, myValue);
    lastChangeTime = millis();
    eepromCommitted = false;
  }
}

void changeAnimationSpeed(int value){
  if(value < 10){
    value = 10;
  }
  if(value > 255){
    value = 255;
  }
  if (myAnimationSpeed != value) {
    myAnimationSpeed = value;
    EEPROM.write(6, myAnimationSpeed&255);
    lastChangeTime = millis();
    eepromCommitted = false;
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

