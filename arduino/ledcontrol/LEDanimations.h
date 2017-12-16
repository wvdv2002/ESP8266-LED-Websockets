

typedef void (*SimplePatternList[])();                        // List of patterns to cycle through.  Each is defined as a separate function below.

//RIPPLE START
int color;
int center = 0;
int step = -1;
int maxSteps = 16;
float fadeRate = 0.8;
int diff;

//background color
uint32_t currentBg = random(256);
uint32_t nextBg = currentBg;

void ripple(void);
void Fire2012(void);
void cylon(void);
void rainbow(void);
void ledsOff(void);
void ledsSolid(void);
void ledsCandle(void);

#define AMOUNT_OF_ANIMATIONS 7
SimplePatternList ledPatterns = {ledsOff,ledsSolid,ledsCandle,ripple, Fire2012, cylon, rainbow};       
String ledPatternNamesList = "Off,Solid,Candle,Ripple,Fire,Cylon,Rainbow";


void one_color_allHSV(int ahue, int abright) {                // SET ALL LEDS TO ONE COLOR (HSV)
  for (int i = 0 ; i < NUM_LEDS; i++ ) {
    leds[i] = CHSV(ahue, 255, abright);
  }
}


void ledsOff(void){}
void ledsOn(void){}
void rainbow(void){}

int flickerLed = 0;
void ledsCandle(void)
{
  static int flickerTime = 200;

  static int flickerValue = 110;
  static int flickerHue = 33;
  static unsigned long currentTime = 0;
  static unsigned long previousTime = 0;
  currentTime = millis();
  ledSet.fadeToBlackBy(1);
  leds[flickerLed] = CHSV(flickerHue, 255, flickerValue);
  if (currentTime - previousTime > flickerTime) {
    flickerValue = 110 + random(-10, +10); //70 works best
    flickerHue = 33; //random(33, 34);
    previousTime = currentTime;
    flickerTime = random(150, 500);
  }  
}


void ledsSolid(void){
  ledSet = CHSV(myHue, mySaturation, myValue);  
}



int wrap(int step) {
  if (step < 0) return NUM_LEDS + step;
  if (step > NUM_LEDS - 1) return step - NUM_LEDS;
  return step;
}

void ripple() {
    if (currentBg == nextBg) {
      nextBg = random(256);
    }
    else if (nextBg > currentBg) {
      currentBg++;
    } else {
      currentBg--;
    }
    for (uint16_t l = 0; l < NUM_LEDS; l++) {
      leds[l] = CHSV(currentBg, 255, 50);         // strip.setPixelColor(l, Wheel(currentBg, 0.1));
    }

    if (step == -1) {
      center = random(NUM_LEDS);
      color = random(256);
      step = 0;
    }

    if (step == 0) {
      leds[center] = CHSV(color, 255, 255);         // strip.setPixelColor(center, Wheel(color, 1));
      step ++;
    }
    else {
      if (step < maxSteps) {
        //Serial.println(pow(fadeRate,step));

        leds[wrap(center + step)] = CHSV(color, 255, pow(fadeRate, step) * 255);     //   strip.setPixelColor(wrap(center + step), Wheel(color, pow(fadeRate, step)));
        leds[wrap(center - step)] = CHSV(color, 255, pow(fadeRate, step) * 255);     //   strip.setPixelColor(wrap(center - step), Wheel(color, pow(fadeRate, step)));
        if (step > 3) {
          leds[wrap(center + step - 3)] = CHSV(color, 255, pow(fadeRate, step - 2) * 255);   //   strip.setPixelColor(wrap(center + step - 3), Wheel(color, pow(fadeRate, step - 2)));
          leds[wrap(center - step + 3)] = CHSV(color, 255, pow(fadeRate, step - 2) * 255);   //   strip.setPixelColor(wrap(center - step + 3), Wheel(color, pow(fadeRate, step - 2)));
        }
        step ++;
      }
      else {
        step = -1;
      }
    }
}
// RIPPLE END

bool gReverseDirection = false;

#define COOLING  100

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 70


void Fire2012()
{
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];
    // Step 1.  Cool down every cell a little
    for ( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for ( int k = NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if ( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160, 255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for ( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if ( gReverseDirection ) {
        pixelnumber = (NUM_LEDS - 1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}
//Fire2012 End


//CYLON START


void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(200);
  }
}

void cylon() {
  static uint8_t hue = 0;
  static int stepCount = 0;
  static int cylon_state = 2;
  //Serial.print("x");
  // First slide the led in one direction
    switch (cylon_state) {
      case 0:
        if (stepCount < NUM_LEDS-1) {
          stepCount++;
          leds[stepCount] = CHSV(hue++, 255, 255);
          fadeall();
        }
        else
        {
          cylon_state = 1;
        }
        break;
      case 1:
        if (stepCount > 0) {
          // Now go in the other direction.
          // Set the i'th led to red
           stepCount--;
          leds[stepCount] = CHSV(hue++, 255, 255);
          fadeall();
        }
        else
        {
          cylon_state = 2;
        }
        break;
      default:
        cylon_state = 0;
        stepCount = 0;
        break;
    }
}
//CYLON END




