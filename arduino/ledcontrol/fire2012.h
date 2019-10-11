
void ledsCandle(void)
{

  static int flickerValue = 110;
  static int flickerHue = 33;
  static int flickerSkip = 0;
  ledSet.fadeToBlackBy(1);
  leds[startindex] = CHSV(flickerHue, 255, flickerValue);
  if(flickerSkip--<0){
    flickerValue = 110 + random(-10, +10); //70 works best
    flickerHue = random(33, 34);
    flickerSkip = random(12, 40);
  }
}

#define COOLING 16
#define SPARKING 60

void Fire2012()
{

  //num_leds = 72
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];
  static byte heat1[NUM_LEDS];
  static byte heat2[NUM_LEDS];
    // Step 1.  Cool down every cell a little
    for ( int i = 0; i < NUM_LEDS; i++) {
      heat1[i] = qsub8( heat1[i],  random8(0, FireCooling)); //((COOLING * 10) / NUM_LEDS) + 2));
      heat2[i] = qsub8( heat2[i],  random8(0, FireCooling)); //((COOLING * 10) / NUM_LEDS) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for ( int k = NUM_LEDS - 1; k >= 2; k--) {
      heat1[k] = (heat1[k - 1] + heat1[k - 2] + heat1[k - 2] ) / 3;
      heat2[k] = (heat2[k - 1] + heat2[k - 2] + heat2[k - 2] ) / 3;
    }

    // Step 3.  Fuse the two heat params
    for ( int i = 0; i < NUM_LEDS; i++) {
      if (heat1[i] == 0) { heat[i] = heat2[NUM_LEDS-1-i]; }
      else if (heat2[i] == 0) { heat[i] = heat1[i]; } 
      else { heat[i] = ((heat1[i] + heat2[NUM_LEDS-1-i])*2) / 3; }
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if ( random8() < FireSparking ) {
      int y = random8(7);
      heat1[y] = qadd8( heat1[y], random8(160, 255) );
    }

    // Step 4.  Randomly ignite new 'sparks' of heat near the bottom on the other side
    if ( random8() < FireSparking ) {
      int y = random8(7);
      heat2[y] = qadd8( heat2[y], random8(160, 255) );
    }

    // Step 5.  Map from heat cells to LED colors
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
