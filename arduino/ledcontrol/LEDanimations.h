/* This is currently a (working) mess copied together from animation examples over the web
  and I will try to credit everyone and clean it up over time - if you see code here that you wrote
  and you want to be credited, please let me know - I do not wish to cause offense. */

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


int wrap(int step) {
  if (step < 0) return NUM_LEDS + step;
  if (step > NUM_LEDS - 1) return step - NUM_LEDS;
  return step;
}


void one_color_allHSV(int ahue, int abright) {                // SET ALL LEDS TO ONE COLOR (HSV)
  for (int i = 0 ; i < NUM_LEDS; i++ ) {
    leds[i] = CHSV(ahue, 255, abright);
  }
}

void ripple() {
  static int last_time = millis();
  if ((millis() - last_time) > 300) //if the amount of milliseconds difference is too large, reset the difference.
  {
    last_time = millis() + (50 + 1);
  }
  if ((millis() - last_time) > 50)
  {
    last_time += 50;


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

    LEDS.show();
  }
}
// RIPPLE END

// Fire2012 Start
#define FRAMES_PER_SECOND 25

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
  static int last_time = millis();
  if ((millis() - last_time) > 300) //if the amount of milliseconds difference is too large, reset the difference.
  {
    last_time = millis() + (1000 / FRAMES_PER_SECOND + 1);
  }
  if ((millis() - last_time) > (1000 / FRAMES_PER_SECOND))
  {
    last_time += (1000 / FRAMES_PER_SECOND);


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
    FastLED.show(); // display this frame
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
  static int i = 0;
  static int cylon_state = 2;
  static int last_time = millis();
  //Serial.print("x");
  // First slide the led in one direction
  if ((millis() - last_time) > 200) //if the amount of milliseconds difference is too large, reset the difference.
  {
    last_time = millis() - 30;
  }
  if ((millis() - last_time) > 25)
  {
    last_time += 25;
    switch (cylon_state) {
      case 0:
        if (i < NUM_LEDS) {
          i++;
          // Set the i'th led to red
          leds[i] = CHSV(hue++, 255, 255);
          // Show the leds
          FastLED.show();
          // now that we've shown the leds, reset the i'th led to black
          // leds[i] = CRGB::Black;
          fadeall();
          // Wait a little bit before we loop around and do it again

        }
        else
        {
          cylon_state = 1;
        }
        break;
      case 1:
        if (i > 0) {
          // Now go in the other direction.
          // Set the i'th led to red
          leds[i - 1] = CHSV(hue++, 255, 255);
          // Show the leds
          FastLED.show();
          // now that we've shown the leds, reset the i'th led to black
          // leds[i] = CRGB::Black;
          fadeall();
          // Wait a little bit before we loop around and do it again
        }
        else
        {
          cylon_state = 2;
        }
        break;
      default:
        cylon_state = 0;
        last_time = millis();
        i = 0;
        break;
    }
  }
}
//CYLON END
