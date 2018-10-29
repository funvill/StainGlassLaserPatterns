/**
 * circlePie - Stainglass window 
 * Written by: Steven Smethurst 
 * More info https://blog.abluestar.com/projects/2018-stained-glass-window 
 */ 
#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"

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
#define NUM_LEDS    38
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120


const unsigned char PATTERN_OUTSIDE_PETTLES[] = { 6,10,23,30,26,13 };
const unsigned char PATTERN_OUTSIDE_PETTLES_LED_COUNT = 6 ;

const unsigned char PATTERN_INSIDE_PETTLES[] = { 11,12,17,25,24,19 };
const unsigned char PATTERN_INSIDE_PETTLES_LED_COUNT = 6 ;

const unsigned char PATTERN_CONNERS[] = { 0,1,8,7,11,  9,21,20,22,19,  32,33,34,31,24,  35,36,28,29,25,  27,15,14,16,17,  2,3,4,5,12 };
const unsigned char PATTERN_CONNERS_LED_ELEMENT_SIZE = 5 ;
const unsigned char PATTERN_CONNERS_LED_ELEMENT_COUNT = 6 ;
const unsigned char PATTERN_CONNERS_LED_COUNT = PATTERN_CONNERS_LED_ELEMENT_SIZE * PATTERN_CONNERS_LED_ELEMENT_COUNT ;

const unsigned char PATTERN_RING_ONE[] = {0,3,15,36,33,21 };
const unsigned char PATTERN_RING_ONE_LED_COUNT = 6;

const unsigned char PATTERN_RING_TWO[] = { 8,1,  2,4,  14,27,  28,35,  34,32,  22,9 };
const unsigned char PATTERN_RING_TWO_LED_COUNT = 12;

const unsigned char PATTERN_RING_THREE[] = { 6,5,13,16,26,29,30,31,23,20,10,7 };
const unsigned char PATTERN_RING_THREE_LED_COUNT = 12;





void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);  

  // Set on startup 
  fill_solid( leds, NUM_LEDS, CRGB(255,0,0));
  FastLED.show();
  delay(1000);
  fill_solid( leds, NUM_LEDS, CRGB(0,255,0));
  FastLED.show();
  delay(1000);
  fill_solid( leds, NUM_LEDS, CRGB(0,0,255));
  FastLED.show();
  delay(1000);


}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { Rings, Clock, PatternConer, /*rainbowWithGlitter, sinelon, juggle,*/ confetti, Rings, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void loop()
{
  // PatternConer() ; 

  
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

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}


void PatternConers( unsigned char conersOffset ) {
  if( conersOffset >= PATTERN_CONNERS_LED_ELEMENT_COUNT ) {
    conersOffset = PATTERN_CONNERS_LED_ELEMENT_COUNT ; 
  }

  Serial.print("PatternConer: " + String(conersOffset) + " ( " );
      

  for(unsigned int offset = conersOffset * PATTERN_CONNERS_LED_ELEMENT_SIZE ; 
                    offset < (conersOffset + 1 ) * PATTERN_CONNERS_LED_ELEMENT_SIZE; 
                    offset++ ) 
  {
    Serial.print( String(offset) ); 
    Serial.print( " = ");
    Serial.print( PATTERN_CONNERS[offset] );
    Serial.print( ", "); 
    
    leds[PATTERN_CONNERS[offset]] = CHSV(gHue, 200, 255);
  }
  Serial.println( " )");
}


void PatternConer() {
  fadeToBlackBy(leds, NUM_LEDS, 5);

  const unsigned short SPEED = 200;
  static unsigned long nextUpdated = 0;
  if (nextUpdated < millis()) {
      nextUpdated = millis() + SPEED;

      static unsigned char lastOffset = 0;
      lastOffset++; 
      if( lastOffset >= PATTERN_CONNERS_LED_ELEMENT_COUNT ) {
        lastOffset = 0 ; 
      }      
      PatternConers(lastOffset);       
  }

  // Ranbow 
  leds[6] = CHSV(gHue + 100 , 200, 255);
  leds[10] = CHSV(gHue + 100 , 200, 255);
  leds[23] = CHSV(gHue + 100 , 200, 255);
  leds[30] = CHSV(gHue + 100 , 200, 255);
  leds[26] = CHSV(gHue + 100 , 200, 255);
  leds[13] = CHSV(gHue + 100 , 200, 255);
  leds[18] = CHSV(gHue + 100 , 200, 255);
}


void Clock() {
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  leds[PATTERN_INSIDE_PETTLES[0]] = CHSV(gHue + 100 , 200, 255);
  leds[PATTERN_INSIDE_PETTLES[1]] = CHSV(gHue + 100 , 200, 255);
  leds[PATTERN_INSIDE_PETTLES[2]] = CHSV(gHue + 100 , 200, 255);
  leds[PATTERN_INSIDE_PETTLES[3]] = CHSV(gHue + 100 , 200, 255);
  leds[PATTERN_INSIDE_PETTLES[4]] = CHSV(gHue + 100 , 200, 255);
  leds[PATTERN_INSIDE_PETTLES[5]] = CHSV(gHue + 100 , 200, 255);
  
}

void Rings() {
  for( unsigned char offset = 0 ; offset < PATTERN_RING_ONE_LED_COUNT ; offset++ ) {
    leds[PATTERN_RING_ONE[offset]] = CHSV(gHue, 200, 255);  
  }

  for( unsigned char offset = 0 ; offset < PATTERN_RING_TWO_LED_COUNT ; offset++ ) {
    leds[PATTERN_RING_TWO[offset]] = CHSV(gHue + 120, 200, 255);  
  }

  for( unsigned char offset = 0 ; offset < PATTERN_RING_THREE_LED_COUNT ; offset++ ) {
    leds[PATTERN_RING_THREE[offset]] = CHSV(gHue + 180, 200, 255);  
  }
  
  for( unsigned char offset = 0 ; offset < PATTERN_INSIDE_PETTLES_LED_COUNT ; offset++ ) {
    leds[PATTERN_INSIDE_PETTLES[offset]] = CHSV(gHue + 60, 200, 255);  
  }

  leds[18] = CHSV(gHue + 200, 200, 255);  
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

