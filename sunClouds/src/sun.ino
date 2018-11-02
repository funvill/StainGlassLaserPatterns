/**
 * Sun Clouds - Stainglass window 
 * Written by: Steven Smethurst 
 * More info https://blog.abluestar.com/projects/2018-stained-glass-window 
 */ 
const char APP_NAME[] = "Sun Clouds";
const uint16_t APP_VERSION_MAJOR = 0;
const uint16_t APP_VERSION_MINOR = 0;
const uint16_t APP_VERSION_PATCH = 3;

#define FASTLED_ALLOW_INTERRUPTS 0
#include <Arduino.h>
#include "FastLED.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    D6
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
#define NUM_LEDS    48
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// Settings 
const uint16_t SETTING_TIME_FOR_EACH_PATTERN = 30 ; 

// Patterns 
const unsigned char PATTERN_SUN[] = {0,1,2,3,4,5,6,7,8,9,10,11,27};
const unsigned char PATTERN_RAYS[] = {12,13,15,16,18,19,20,21,22,23,25,26,28,30,32,33,35,36,37,38,39,40,42,43,44,46,47};
const unsigned char PATTERN_CLOUDS[] = {14,17,24,29,31,34,41,45};

#define PATTERN_RING_ONE PATTERN_SUN 
const uint8_t PATTERN_RING_TWO[] = {28,26,36,21,30,35,42,44,38,37,43,13,18,19,20,25,12,47};
#define PATTERN_RING_THREE PATTERN_CLOUDS 
const uint8_t PATTERN_RING_FOUR[] = {15,16,22,23,32,33,39,40,46};

const uint8_t PATTERN_SIXTH[] = { 15,13,16,18,19,22,21,20,14,17,14,12,14,
                                  23,25,26,32,30,28,33,35,36,31,24,29,34,
                                  39,38,37,40,42,43,46,44,34,41,45,47,45,
                                   9, 8, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                   1, 4, 7,10,27, 1, 1, 1, 1, 1, 1, 1, 1,
                                   0, 5, 6,11, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);  
  PrintStartUpInfo(); 
  TestPattern(); 
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { Sun, Sixth, Rings, Groups, /* Triangles, Clock,  */ confetti, rainbow, sinelon, bpm, juggle };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
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
  EVERY_N_SECONDS( SETTING_TIME_FOR_EACH_PATTERN ) { nextPattern(); } // change patterns periodically
}

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  Serial.print("[");
  Serial.print(millis());
  Serial.print("] FYI: Changing pattern to offset: ");
  Serial.print(gCurrentPatternNumber);
  Serial.print("\n");
}

void PrintStartUpInfo() {
  Serial.print("\nName: ");
  Serial.print(APP_NAME);
  Serial.print(", Version: ");
  Serial.print(APP_VERSION_MAJOR, DEC);
  Serial.print(".");
  Serial.print(APP_VERSION_MINOR, DEC);
  Serial.print(".");
  Serial.print(APP_VERSION_PATCH, DEC);

  Serial.print("\nPattern Count: ");
  Serial.print(ARRAY_SIZE( gPatterns));
  Serial.print(", Pattern length (ms): ");
  Serial.print(SETTING_TIME_FOR_EACH_PATTERN);
  Serial.print("\n");
}


void TestPattern(){

  Serial.print("FYI: Pattern: TestPattern\n");
  
  Serial.print("Red\n");
  fill_solid( leds, NUM_LEDS, CRGB(255,0,0));
  FastLED.show();
  delay(1000);
  Serial.print("Blue\n");
  fill_solid( leds, NUM_LEDS, CRGB(0,255,0));
  FastLED.show();
  delay(1000);
  Serial.print("Green\n");
  fill_solid( leds, NUM_LEDS, CRGB(0,0,255));
  FastLED.show();
  delay(1000);
}

// Helper functions 
// ----------------------------------------------------------------------------

void PatternRotateThoughSubSections( unsigned long &nextUpdate, uint16_t &segmentOffset, const uint8_t * pattern, uint32_t patternSize, uint32_t subsectionSize, long speed, int8_t direction, uint8_t hue) {


/*   Serial.print("[PatternRotateThoughSubSections nextUpdate=");
  Serial.print(nextUpdate);
  Serial.print(" segmentOffset=");
  Serial.print(segmentOffset);
  Serial.print(" patternSize=");
  Serial.print(patternSize);
  Serial.print(" subsectionSize=");
  Serial.print(subsectionSize);
  Serial.print(" speed=");
  Serial.print(speed);
  Serial.print(" direction=");
  Serial.print(direction);
  Serial.print(" hue=");
  Serial.print(hue);
  Serial.print("]");
 */
  if( patternSize <= 0 || subsectionSize <= 0 || speed <= 0 || subsectionSize > patternSize) {
    return ; 
  }

  // Only update the segments on a timer. 
  if (nextUpdate < millis()) {
    nextUpdate = millis() + speed;

    // Get the amount of sub sections for the entire pattern 
    uint16_t COUNT_SUBSECTIONS = patternSize / subsectionSize;

    // increase the offset 
    segmentOffset += direction;
    uint16_t segmentStart = (segmentOffset % COUNT_SUBSECTIONS) ; 

    // Loop thought the LEDS in the segment and set them to the color. 
    for (uint32_t offsetLED = segmentStart * subsectionSize; offsetLED < (segmentStart * subsectionSize ) + subsectionSize; offsetLED++) {
        leds[pattern[offsetLED]] = CHSV(hue, 200, 255);
    }
  }
}

void PatternSetColor(const uint8_t * pattern, uint32_t patternSize, uint8_t hue ) {
  for (uint32_t offsetLED = 0; offsetLED < patternSize; offsetLED++) {
    leds[pattern[offsetLED]] = CHSV(hue, 200, 255);
  }
}

void PatternSegmentRainbow(const uint8_t * pattern, uint32_t patternSize, uint16_t segmentsSize, uint8_t hue) {
  uint16_t segmentsCount = patternSize / segmentsSize ; 
  for( uint16_t segmentOffset = 0 ; segmentOffset < segmentsCount ; segmentOffset++) {
    PatternSetColor( pattern + ( segmentOffset * segmentsSize), segmentsSize, hue + (255 / segmentsCount) * segmentOffset) ;
  }
}



// Default patterns 
// ----------------------------------

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



// Custom patterns. 
// ----------------------------------------------------------------------------
void Groups() {
  PatternSetColor( PATTERN_SUN, ARRAY_SIZE(PATTERN_SUN), gHue + (255/3) * 0) ;
  PatternSetColor( PATTERN_RAYS, ARRAY_SIZE(PATTERN_RAYS), gHue + (255/3) * 1) ;
  PatternSetColor( PATTERN_CLOUDS, ARRAY_SIZE(PATTERN_CLOUDS), gHue + (255/3) * 2) ;
}

/*
void Triangles() {
  fadeToBlackBy(leds, NUM_LEDS, 6);
  static unsigned long nextUpdate[1]  ;
  static uint16_t segmentOffset[1] ; 
  PatternRotateThoughSubSections( nextUpdate[0], segmentOffset[0], PATTERN_TRIANGLES , ARRAY_SIZE(PATTERN_TRIANGLES ), 12, 700, 1, gHue);
}
*/

void Clock() {
  fadeToBlackBy(leds, NUM_LEDS, 6);

  static unsigned long nextUpdate[5] ;
  static uint16_t segmentOffset[5] ; 
  static bool startup = true; 
  if( startup ) {
    startup = false;
    for( uint8_t offset = 0 ; offset < 5 ; offset++ ) {
      nextUpdate[offset] = 0 ; 
      segmentOffset[offset] = 0 ; 
    }
  }

  PatternRotateThoughSubSections( nextUpdate[0], segmentOffset[0], PATTERN_RING_ONE , ARRAY_SIZE(PATTERN_RING_ONE), 1, 900, 1, gHue + (255/5) * 0);   
  PatternRotateThoughSubSections( nextUpdate[1], segmentOffset[1], PATTERN_RING_TWO , ARRAY_SIZE(PATTERN_RING_TWO), 1, 150, 1, gHue + (255/5) * 1);   
  PatternRotateThoughSubSections( nextUpdate[2], segmentOffset[2], PATTERN_RING_THREE , ARRAY_SIZE(PATTERN_RING_THREE), 1, 75, 1, gHue + (255/5) * 2);   
  PatternRotateThoughSubSections( nextUpdate[3], segmentOffset[3], PATTERN_RING_FOUR , ARRAY_SIZE(PATTERN_RING_FOUR), 1, 50, 1, gHue + (255/5) * 3);   
  // PatternRotateThoughSubSections( nextUpdate[4], segmentOffset[4], PATTERN_RING_FIVE , ARRAY_SIZE(PATTERN_RING_FIVE), 1, 38, 1, gHue + (255/5) * 4);   
}

void Rings() {
  fadeToBlackBy(leds, NUM_LEDS, 6);
  PatternSetColor( PATTERN_RING_ONE,   ARRAY_SIZE(PATTERN_RING_ONE), gHue + (255/4) * 0);
  PatternSetColor( PATTERN_RING_TWO,   ARRAY_SIZE(PATTERN_RING_TWO), gHue + (255/4) * 1);
  PatternSetColor( PATTERN_RING_THREE, ARRAY_SIZE(PATTERN_RING_THREE), gHue + (255/4) * 2);
  PatternSetColor( PATTERN_RING_FOUR,  ARRAY_SIZE(PATTERN_RING_FOUR), gHue + (255/4) * 3);
}

void Sixth() {  
  fadeToBlackBy(leds, NUM_LEDS, 6);
  static unsigned long nextUpdate[1];
  static uint16_t segmentOffset[1]; 
  PatternRotateThoughSubSections( nextUpdate[0], segmentOffset[0], PATTERN_SIXTH , ARRAY_SIZE(PATTERN_SIXTH), 13, 200, 1, gHue);   
}

void Sun() {
  static uint8_t hue = 0 ; 
  static int8_t direction = 1 ; 
  static long nextUpdate = 0 ; 
  const uint8_t SPEED = 10 ; 

  // Rotate up and down between 0 and 64 
  if (nextUpdate < millis()) {
    nextUpdate = millis() + SPEED;

    hue += direction;
    if( hue > 32 || hue <= 0 ) {
      direction = -1 * direction ; 
    }
  }
 
  PatternSetColor( PATTERN_SUN, ARRAY_SIZE(PATTERN_SUN), 244 + hue ) ;
  PatternSetColor( PATTERN_RAYS, ARRAY_SIZE(PATTERN_RAYS), 32 + hue ) ;
  PatternSetColor( PATTERN_CLOUDS, ARRAY_SIZE(PATTERN_CLOUDS), 128 + 20 + hue) ;
}