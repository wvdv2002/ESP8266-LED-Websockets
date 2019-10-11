/*
*
 * seirlight led lighting effects for FastLED.
 * 
 *       By: Andrew Tuline
 *    Date: March, 2017
 *      URL: www.tuline.com
 *    Email: atuline@gmail.com
 *   GitHub: https://github.com/atuline
 * Pastebin: http://pastebin.com/u/atuline
 *  Youtube: https://www.youtube.com/user/atuline/videos
 * Changed by: Wvdv2002 
 * 
 * CAUTION ************************************************************************************************************************************************
 * 
 * Before attempting to compile this routine, make sure you are already comfortable modifying Arduino Code and FastLED code in particular. In addition, you
 * should already be able to download, install and use 3rd party libraries. If you are a beginner, this is NOT the code you're looking for.
 * 
 * ********************************************************************************************************************************************************
 * 
 * 
 * Introduction
 * 
 * This is a significant re-write of my previous aalight program and provides the following:
 * 
 * - Uses an Arduino microcontroller.
 * - Uses the FastLED display library.
 * - Supports multiple display sequences, most with support for multiple settings.
 * - Supports IR (Infra Red) communications for control of the display sequences.
 * - Supports keyboard communications for control of the display sequences.
 * - Supports a button for 3 functions.
 * - Can support APA102 and WS2801 with IR communications.
 * - Can support WS2812 if NOT using keyboard or IR communications.
 * - Can save information in EEPROM.
 * 
 * 
*/

uint8_t maxMode = 40;                                         // Maximum number of modes.
const char* ledPatternNamesList = "Off,Solid,White,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38";


/*------------------------------------------------------------------------------------------
--------------------------------------- Start of variables ---------------------------------
------------------------------------------------------------------------------------------*/


#define qsubd(x, b)  ((x>b)?wavebright:0)                     // A digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b)  ((x>b)?x-b:0)                            // Unsigned subtraction macro. if result <0, then => 0.

#define SEIRLIGHT_VERSION 102


#include "FastLED.h"                                          // https://github.com/FastLED/FastLED

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif



// Serial variables
byte inbyte;                                                  // Serial input byte
int thisarg;                                                  // Serial input argument

#define MAX_LEDS 255                                          // Maximum number of LED's defined (at compile time).


// Initialize changeable global variables.
uint8_t max_bright = 255;                                     // Overall brightness definition. It can be changed on the fly.

CRGBPalette16 currentPalette;                                 // Use palettes instead of direct CHSV or CRGB assignments
CRGBPalette16 targetPalette;                                  // Also support smooth palette transitioning
TBlendType    currentBlending;                                // NOBLEND or LINEARBLEND

CRGB currentSolid;

uint8_t ledMode = 0;                                              // Starting mode is typically 0.
uint8_t updateNeeded = 0;                                     // tell main loop if leds need to be updated
uint8_t demorun = 0;                                          // 0 = regular mode, 1 = demo mode, 2 = shuffle mode.
uint8_t demotime = 10;                                        // Set the length of the demo timer.


// Generic/shared routine variables ----------------------------------------------------------------------
uint8_t allfreq = 32;                                         // You can change the frequency, thus overall width of bars.
uint8_t bgclr = 0;                                            // Generic background colour
uint8_t bgbri = 0;                                            // Generic background brightness
bool    glitter = 0;                                          // Glitter flag
uint8_t palchg;                                               // 0=no change, 1=similar, 2=random
uint8_t startindex = 0;
uint8_t thisbeat;                                             // Standard beat
uint8_t thisbright = 0;                                       // Standard brightness
uint8_t thiscutoff = 192;                                     // You can change the cutoff value to display this wave. Lower value = longer wave.
int thisdelay = 10;                                            // Standard delay
uint8_t thisdiff = 1;                                         // Standard palette jump
bool    thisdir = 0;                                          // Standard direction
uint8_t thisfade = 224;                                       // Standard fade rate
uint8_t thishue = 0;                                          // Standard hue
uint8_t thisindex = 0;                                        // Standard palette index
uint8_t thisinc = 1;                                          // Standard incrementer
int     thisphase = 0;                                        // Standard phase change
uint8_t thisrot = 1;                                          // You can change how quickly the hue rotates for this wave. Currently 0.
uint8_t thissat = 255;                                        // Standard saturation
int8_t  thisspeed = 4;                                        // Standard speed change
uint8_t wavebright = 255;                                     // You can change the brightness of the waves/bars rolling across the screen.

uint8_t xd[MAX_LEDS];                                         // arrays for the 2d coordinates of any led
uint8_t yd[MAX_LEDS];

long summ=0;



extern const TProgmemRGBGradientPalettePtr gGradientPalettes[]; // These are for the fixed palettes in gradient_palettes.h
extern const uint8_t gGradientPaletteCount;                     // Total number of fixed palettes to display.
uint8_t gCurrentPaletteNumber = 0;                              // Current palette number from the 'playlist' of color palettes
uint8_t currentPatternIndex = 0;                                // Index number of which pattern is current



// Display functions -----------------------------------------------------------------------

// Support functions
#include "addglitter.h"
#include "make_palettes.h"

// Display routines
#include "circnoise_pal_1.h"
#include "circnoise_pal_2.h"
#include "circnoise_pal_3.h"
#include "circnoise_pal_4.h"
#include "confetti_pal.h"
#include "gradient_palettes.h"
#include "juggle_pal.h"
#include "matrix_pal.h"
#include "noise16_pal.h"
#include "noise8_pal.h"
#include "one_sin_pal.h"
#include "rainbow_march.h"
#include "serendipitous_pal.h"
#include "three_sin_pal.h"
#include "two_sin.h"
#include "fire2012.h"



void strobe_mode(uint8_t newMode, bool mc);
void demo_check(void);

/*------------------------------------------------------------------------------------------
--------------------------------------- Start of code --------------------------------------
------------------------------------------------------------------------------------------*/

void ledAnimationsSetup(void) {
  random16_set_seed(4832);                                                        // Awesome randomizer of awesomeness
  int ranstart = random16();

  currentPalette  = CRGBPalette16(CRGB::Black);
  targetPalette   = RainbowColors_p;
  currentBlending = LINEARBLEND;

  // This is for Stefan Petrick's Circular Noise routines
  for (uint8_t i = 0; i < NUM_LEDS; i++) {        // precalculate the lookup-tables:
    uint8_t angle = (i * 256) / NUM_LEDS;         // on which position on the circle is the led?
    xd[i] = cos8( angle );                         // corresponding x position in the matrix
    yd[i] = sin8( angle );                         // corresponding y position in the matrix
  }
  
  strobe_mode(ledMode, 1);                                                        // Initialize the first sequence
  
} // setup()



//------------------MAIN LOOP---------------------------------------------------------------
void ledAnimationsLoop() {
  demo_check();                                                             // If we're in demo mode, check the timer to see if we need to increase the strobe_mode value.

  EVERY_N_MILLISECONDS(50) {                                                 // Smooth palette transitioning runs continuously.
    uint8_t maxChanges = 24; 
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);   
  }

  EVERY_N_SECONDS(5) {                                                        // If selected, change the target palette to a random one every 5 seconds.
    if (palchg == 1) SetupSimilar4Palette();
    if (palchg == 2) SetupRandom4Palette();
    if (palchg == 3) SetupRandom16Palette();
}

  EVERY_N_MILLIS_I(thistimer, thisdelay) {                                    // Sets the original delay time.
    thistimer.setPeriod(thisdelay);                                           // This is how you update the delay value on the fly.
    strobe_mode(ledMode, 0);                                                  // Strobe to display the current sequence, but don't initialize the variables, so mc=0;
    if (ledMode > 2) updateNeeded = 1;                                        //update only when ledmode > 2
  }

  if(glitter) addglitter(10);                                                 // If the glitter flag is set, let's add some.
  
} // loop()

void ledAnimationsChangedAnimation(int newMode){
  if (newMode != ledMode){
    strobe_mode(newMode, 1);
    ledMode = newMode;
    updateNeeded = 1;
  }
}

void ledAnimationsChangedAnimationSpeed(int aSpeed){
  if (thisdelay != aSpeed){
    thisdelay=aSpeed;
  }
}

void ledAnimationsSetSolidColor(CRGB aNewSolid){
  currentSolid = aNewSolid;
  strobe_mode(ledMode, 1);
}


//-------------------OTHER ROUTINES----------------------------------------------------------
void strobe_mode(uint8_t newMode, bool mc){                   // mc stands for 'Mode Change', where mc = 0 is strobe the routine, while mc = 1 is change the routine

  if(mc) {
    fill_solid(leds,NUM_LEDS,CRGB(0,0,0));                    // Clean up the array for the first time through. Don't show display though, so you may have a smooth transition.
    palchg=0;
  }
   
  switch (newMode) {                                          // First time through a new mode, so let's initialize the variables for a given display.

    case  0: if(mc) {fill_solid(leds,NUM_LEDS,CHSV(50,50,0)); myWhiteLedValue = 0; palchg=0;}  break;                     // All off, not animated.
    case  1: if(mc) {fill_solid(leds, NUM_LEDS,currentSolid); palchg=0;} break;              // All on, not animated.
    case  2: if(mc) {fill_solid(leds,NUM_LEDS,CHSV(50,50,0)); myWhiteLedValue = 50; palchg=0;} break;                     // All white, not animated.
    case  3: if(mc) {thisdelay=80; fill_solid(leds,NUM_LEDS,CHSV(50,50,0)); myWhiteLedValue = 0;}Fire2012();break;
    case  4: if(mc) {thisdelay=2; startindex=random(0,NUM_LEDS-1); myWhiteLedValue = 0; palchg=0;} ledsCandle(); break;
    case  5: if(mc) {thisdelay=10; allfreq=2; thisspeed=1; thatspeed=1; thishue=0; thathue=128; thisdir=0; thisrot=1; thatrot=1; thiscutoff=128; thatcutoff=192; myWhiteLedValue = 0; palchg=0;} two_sin(); break;
    case  6: if(mc) {thisdelay=20; targetPalette=RainbowColors_p; allfreq=4; bgclr=0; bgbri=0; thisbright=255; startindex=64; thisinc=2; thiscutoff=224; thisphase=0; thiscutoff=224; thisrot=0; thisspeed=4; wavebright=255; palchg=0;} one_sin_pal(); break;
    case  7: if(mc) {thisdelay=10; targetPalette = PartyColors_p; palchg=2;} noise8_pal(); break;
    case  8: if(mc) {thisdelay=10; allfreq=4; thisspeed=-1; thatspeed=0; thishue=64; thathue=192; thisdir=0; thisrot=0; thatrot=0; thiscutoff=64; thatcutoff=192; palchg=0;} two_sin(); break;
    case  9: if(mc) {thisdelay=20; targetPalette=RainbowColors_p; allfreq=10; bgclr=64; bgbri=4; thisbright=255; startindex=64; thisinc=2; thiscutoff=224; thisphase=0; thiscutoff=224; thisrot=0; thisspeed=4; wavebright=255; palchg=0;} one_sin_pal(); break;
    case 10: if(mc) {thisdelay=10; numdots=2; targetPalette=PartyColors_p; thisfade=16; thisbeat=8; thisbright=255; thisdiff=64; palchg=0;} juggle_pal(); break;
    case 11: if(mc) {thisdelay=40; targetPalette = LavaColors_p; thisindex=128; thisdir=1; thisrot=0; thisbright=255; bgclr=200; bgbri=6; palchg=0;} matrix_pal(); break;
    case 12: if(mc) {thisdelay=10; allfreq=6; thisspeed=2; thatspeed=3; thishue=96; thathue=224; thisdir=1; thisrot=0; thatrot=0; thiscutoff=64; thatcutoff=64; palchg=0;} two_sin(); break;
    case 13: if(mc) {thisdelay=20; targetPalette=RainbowColors_p; palchg=0; allfreq=16; bgclr=0; bgbri=0; thisbright=255; startindex=64; thisinc=2; thiscutoff=224; thisphase=0; thiscutoff=224; thisrot=0; thisspeed=4; wavebright=255;} one_sin_pal(); break;
    case 14: if(mc) {thisdelay=50; mul1=5; mul2=8; mul3=7; palchg=0;} three_sin_pal(); break;
    case 15: if(mc) {thisdelay=10; targetPalette=ForestColors_p; palchg=0;} serendipitous_pal(); break;
    case 16: if(mc) {thisdelay=20; targetPalette=LavaColors_p; palchg=0; allfreq=8; bgclr=0; bgbri=4; thisbright=255; startindex=64; thisinc=2; thiscutoff=224; thisphase=0; thiscutoff=224; thisrot=0; thisspeed=4; wavebright=255;} one_sin_pal(); break;
    case 17: if(mc) {thisdelay=10; allfreq=20; thisspeed=2; palchg=0; thatspeed=-1; thishue=24; thathue=180; thisdir=1; thisrot=0; thatrot=1; thiscutoff=64; thatcutoff=128;} two_sin(); break;
    case 18: if(mc) {thisdelay=50; targetPalette = PartyColors_p; palchg=0; thisindex=64; thisdir=0; thisrot=1; thisbright=255; bgclr=100; bgbri=10;} matrix_pal(); break;
    case 19: if(mc) {thisdelay=10; targetPalette = OceanColors_p; palchg=1;} noise8_pal(); break;
    case 20: if(mc) {thisdelay=10; targetPalette=PartyColors_p; palchg=0;} circnoise_pal_2(); break;
    case 21: if(mc) {thisdelay=20; allfreq=10; thisspeed=1; thatspeed=-2; thishue=48; thathue=160; thisdir=0; thisrot=1; thatrot=-1; thiscutoff=128; thatcutoff=192;} two_sin(); break;
    case 22: if(mc) {thisdelay=50; mul1=6; mul2=9; mul3=11;} three_sin_pal(); break;
    case 23: if(mc) {thisdelay=10; thisdir=1; thisrot=1; thisdiff=1;} rainbow_march(); break;
    case 24: if(mc) {thisdelay=10; thisdir=1; thisrot=2; thisdiff=10;} rainbow_march(); break;
    case 25: if(mc) {thisdelay=20; hxyinc = random16(1,15); octaves=random16(1,3); hue_octaves=random16(1,5); hue_scale=random16(10, 50);  x=random16(); xscale=random16(); hxy= random16(); hue_time=random16(); hue_speed=random16(1,3); x_speed=random16(1,30);} noise16_pal(); break;
    case 26: if(mc) {thisdelay=20; targetPalette=OceanColors_p; allfreq=6; bgclr=0; bgbri=0; thisbright=255; startindex=64; thisinc=2; thiscutoff=224; thisphase=0; thiscutoff=224; thisrot=0; thisspeed=4; wavebright=255;} one_sin_pal(); break;
    case 27: if(mc) {thisdelay=10; targetPalette=OceanColors_p;} circnoise_pal_4(); break;
    case 28: if(mc) {thisdelay=20; targetPalette = PartyColors_p; thisinc=1; thishue=192; thissat=255; thisfade=2; thisdiff=32; thisbright=255;} confetti_pal(); break;
    case 29: if(mc) {thisdelay=10; thisspeed=2; thatspeed=3; thishue=96; thathue=224; thisdir=1; thisrot=1; thatrot=2; thiscutoff=128; thatcutoff=64;} two_sin(); break;
    case 30: if(mc) {thisdelay=30; targetPalette = ForestColors_p; thisindex=192; thisdir=0; thisrot=0; thisbright=255; bgclr=50; bgbri=0;} matrix_pal(); break;
    case 31: if(mc) {thisdelay=20; targetPalette=RainbowColors_p; allfreq=20; bgclr=0; bgbri=0; thisbright=255; startindex=64; thisinc=2; thiscutoff=224; thisphase=0; thiscutoff=224; thisrot=0; thisspeed=4; wavebright=255;} one_sin_pal(); break;
    case 32: if(mc) {thisdelay=20; targetPalette = LavaColors_p; thisinc=2; thishue=128; thisfade=8; thisdiff=64; thisbright=255;} confetti_pal(); break;
    case 33: if(mc) {thisdelay=10; targetPalette=PartyColors_p;} circnoise_pal_3(); break;
    case 34: if(mc) {thisdelay=30; SetupSimilar4Palette(); allfreq=4; bgclr=64; bgbri=4; thisbright=255; startindex=64; thisinc=2; thiscutoff=224; thisphase=0; thiscutoff=128; thisrot=1; thisspeed=8; wavebright=255;} one_sin_pal(); break;
    case 35: if(mc) {thisdelay=50; mul1=3; mul2=4; mul3=5;} three_sin_pal(); break;
    case 36: if(mc) {thisdelay=10; thisdir=-1; thisrot=1; thisdiff=5;} rainbow_march(); break;
    case 37: if(mc) {thisdelay=10; targetPalette=PartyColors_p;} circnoise_pal_1(); break;
    case 38: if(mc) {thisdelay=20; targetPalette = ForestColors_p; thisinc=1; thishue=random8(255); thisfade=1; thisbright=255;} confetti_pal(); break;
    case 39: if(mc) {thisdelay=20; octaves=1; hue_octaves=2; hxy=6000; x=5000; xscale=3000; hue_scale=50; hue_speed=15; x_speed=100;} noise16_pal(); break;
    case 40: if(mc) {thisdelay=10; targetPalette = LavaColors_p; palchg=0;} noise8_pal(); break;
  } // switch newMode
  
} // strobe_mode()



void demo_check(){
  
  if(demorun) {                                                   // Is the demo flag set? If so, let's cycle through them.
    uint8_t secondHand = (millis() / 1000) % (maxMode*demotime);        // Adjust for total time of the loop, based on total number of available modes.
    static uint8_t lastSecond = 99;                               // Static variable, means it's only defined once. This is our 'debounce' variable.
    if (lastSecond != secondHand) {                               // Debounce to make sure we're not repeating an assignment.
      lastSecond = secondHand;
        if(secondHand%demotime==0) {                                     // Every 10 seconds.
          if(demorun == 2) ledMode = random8(0,maxMode); else {
            ledMode = secondHand/demotime;
          }
          strobe_mode(ledMode,1);                            // Does NOT reset to 0.
      } // if secondHand
    } // if lastSecond
  } // if demorun
  
} // demo_check()
// Turtles all the way down.
