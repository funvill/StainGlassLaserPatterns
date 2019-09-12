#include <Arduino.h>
#include <FastLED.h>

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    D6
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    100
CRGB leds[NUM_LEDS];

#define BRIGHTNESS         255
#define FRAMES_PER_SECOND  120

void nextPattern() ;
void rainbow() ;
void rainbowWithGlitter() ;
void addGlitter( fract8 chanceOfGlitter) ;
void confetti() ;
void sinelon();
void bpm();
void juggle() ;
void zoom(); 
void circles(); 

// --------------------------------------------------------------

const uint8_t OUTSIDE_START = 0 ; 
const uint8_t OUTSIDE_END = 24 ; 
const uint8_t MIDDLE_START = 24 ; 
const uint8_t MIDDLE_END = 48 ; 
const uint8_t INSIDE_START = 48 ; 
const uint8_t INSIDE_END = 72 ; 
const uint8_t CENTER = 73 ; 

// --------------------------------------------------------------




uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
  // List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { circles, zoom, juggle, confetti, sinelon, bpm };
/*, rainbow, rainbowWithGlitter,  */

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}




void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {

  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }

}
void zoom() {
  static uint8_t offset = 0 ; 

  // Only update the segments on a timer. 
  static unsigned long nextUpdate = 0 ; 
  if (nextUpdate < millis()) {
    nextUpdate = millis() + 30;

    offset -= 20 ; 
  }


  fill_solid( leds + OUTSIDE_START, OUTSIDE_END - OUTSIDE_START, CHSV(offset, 200, 255));
  fill_solid( leds + MIDDLE_START, MIDDLE_END - MIDDLE_START, CHSV(offset+50, 200, 255));
  fill_solid( leds + INSIDE_START, INSIDE_END - INSIDE_START, CHSV(offset+100, 200, 255));
}

void circles() {

  static uint8_t offset = 0 ; 
  static uint8_t offset2 = (OUTSIDE_END - OUTSIDE_START) / 2  ; 

  static unsigned long nextUpdate = 0 ; 
  if (nextUpdate < millis()) {
    nextUpdate = millis() + 50;

    offset++ ; 
    if( offset > OUTSIDE_END - OUTSIDE_START ) {
      offset = 0 ; 
    }

    offset2++; 
    if( offset2 > OUTSIDE_END - OUTSIDE_START ) {
      offset2 = 0 ; 
    }
  }
  


  fadeToBlackBy( leds, NUM_LEDS, 20);

  leds[OUTSIDE_START + offset] = CHSV(gHue, 200, 255) ; 
  leds[MIDDLE_START + offset] = CHSV(gHue+20, 200, 255) ; 
  leds[INSIDE_START + offset] = CHSV(gHue+40, 200, 255) ; 

  leds[OUTSIDE_START + offset2] = CHSV(gHue+128, 200, 255) ; 
  leds[MIDDLE_START + offset2] = CHSV(gHue+128+20, 200, 255) ; 
  leds[INSIDE_START + offset2] = CHSV(gHue+128+40, 200, 255) ; 

  leds[CENTER] = CHSV(gHue+80, 200, 255) ; 


}

// --------------------------------------------------------------
/*
void clock()
{

  static uint8_t offsetOutside = 0 ; 
  static uint8_t offsetMiddle = 0 ; 
  static uint8_t offsetInside = 0 ; 

  fill_rainbow( leds + OUTSIDE_START , OUTSIDE_END - OUTSIDE_START, gHue, 7);
  fill_rainbow( leds + MIDDLE_START , MIDDLE_END - MIDDLE_START, gHue, 7);
  fill_rainbow( leds + INSIDE_START , INSIDE_END - INSIDE_START, gHue, 7);
}
*/

// --------------------------------------------------------------

void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 30 ) { nextPattern(); } // change patterns periodically
}
