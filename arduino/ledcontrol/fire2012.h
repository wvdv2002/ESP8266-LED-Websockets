

void ledsCandle(void)
{
  static int flickerTime = 200;

  static int flickerValue = 110;
  static int flickerHue = 33;
  static unsigned long currentTime = 0;
  static unsigned long previousTime = 0;
  currentTime = millis();
  leds[startindex] = CHSV(flickerHue, 255, flickerValue);
  if (currentTime - previousTime > flickerTime) {
    flickerValue = 110 + random(-10, +10); //70 works best
    flickerHue = 33; //random(33, 34);
    previousTime = currentTime;
    flickerTime = random(150, 500);
  }  
}

#define COOLING 100
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
//      if ( gReverseDirection ) {
//        pixelnumber = (NUM_LEDS - 1) - j;
//      } else {
        pixelnumber = j;
//      }
      leds[pixelnumber] = color;
    }
}
