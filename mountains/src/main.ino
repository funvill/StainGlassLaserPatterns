/**
 * Mountains - Stainglass window 
 * Written by: Steven Smethurst 
 * More info https://blog.abluestar.com/projects/2018-stained-glass-window 
 */ 
const char APP_NAME[] = "Mountains";
const uint16_t APP_VERSION_MAJOR = 0;
const uint16_t APP_VERSION_MINOR = 0;
const uint16_t APP_VERSION_PATCH = 1;

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
#define NUM_LEDS    30
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// Settings 
const uint16_t SETTING_TIME_FOR_EACH_PATTERN = 30 ; 

const unsigned char PATTERN_MOUNTAINS[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13};
const unsigned char PATTERN_SKY[] = {14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29};

const unsigned char PATTERN_MOUNTAIN_ONE[] = {0,1,2};
const unsigned char PATTERN_MOUNTAIN_TWO[] = {3,4,5};
const unsigned char PATTERN_MOUNTAIN_THREE[] = {6,7,8};
const unsigned char PATTERN_MOUNTAIN_FOUR[] = {9,10};
const unsigned char PATTERN_MOUNTAIN_FIVE[] = {11};
const unsigned char PATTERN_MOUNTAIN_SIX[] = {12};
const unsigned char PATTERN_MOUNTAIN_SEVEN[] = {13};

const unsigned char PATTERN_SKY_LEVEL_ONE[] = {14,21,27,29};
const unsigned char PATTERN_SKY_LEVEL_TWO[] = {15,20,22,26,28};
const unsigned char PATTERN_SKY_LEVEL_THREE[] = {16,19,23,25};
const unsigned char PATTERN_SKY_LEVEL_FOUR[] = {17,18,24};

const unsigned char PATTERN_SIXTH[] = { 6, 7, 8, 5, 4, 0, 0,
                                        0, 4, 3, 1, 2, 0, 0,
                                        29,28,25,23,22,26,27,
                                        17,18,24,19,23,22,22,
                                        14,15,16,20,21,13,22,
                                        10,11,12,13, 9, 9, 9};
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
SimplePatternList gPatterns = { Lighting, Sixth, NorthenLights, NorthenLights, Groups, confetti, rainbow, sinelon, bpm, juggle };

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
        leds[pattern[offsetLED]] = CHSV(hue, 255, 255);
    }
  }
}

void PatternSetColor(const uint8_t * pattern, uint32_t patternSize, uint8_t hue ) {
  for (uint32_t offsetLED = 0; offsetLED < patternSize; offsetLED++) {
    leds[pattern[offsetLED]] = CHSV(hue, 255, 255);
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
  PatternSetColor( PATTERN_SKY_LEVEL_ONE, ARRAY_SIZE(PATTERN_SKY_LEVEL_ONE), gHue + (255/11) * 0) ;
  PatternSetColor( PATTERN_SKY_LEVEL_TWO, ARRAY_SIZE(PATTERN_SKY_LEVEL_TWO), gHue + (255/11) * 1) ;
  PatternSetColor( PATTERN_SKY_LEVEL_THREE, ARRAY_SIZE(PATTERN_SKY_LEVEL_THREE), gHue + (255/11) * 2) ;
  PatternSetColor( PATTERN_SKY_LEVEL_FOUR, ARRAY_SIZE(PATTERN_SKY_LEVEL_FOUR), gHue + (255/11) * 3) ;
  PatternSetColor( PATTERN_MOUNTAIN_ONE, ARRAY_SIZE(PATTERN_MOUNTAIN_ONE), gHue + (255/11) * 4) ;
  PatternSetColor( PATTERN_MOUNTAIN_TWO, ARRAY_SIZE(PATTERN_MOUNTAIN_TWO), gHue + (255/11) * 5) ;
  PatternSetColor( PATTERN_MOUNTAIN_THREE, ARRAY_SIZE(PATTERN_MOUNTAIN_THREE), gHue + (255/11) * 6) ;
  PatternSetColor( PATTERN_MOUNTAIN_FOUR, ARRAY_SIZE(PATTERN_MOUNTAIN_FOUR), gHue + (255/11) * 7) ;
  PatternSetColor( PATTERN_MOUNTAIN_FIVE, ARRAY_SIZE(PATTERN_MOUNTAIN_FIVE), gHue + (255/11) * 8) ;
  PatternSetColor( PATTERN_MOUNTAIN_SIX, ARRAY_SIZE(PATTERN_MOUNTAIN_SIX), gHue + (255/11) * 9) ;
  PatternSetColor( PATTERN_MOUNTAIN_SEVEN, ARRAY_SIZE(PATTERN_MOUNTAIN_SEVEN), gHue + (255/11) * 10) ;
}

/*
void Triangles() {
  fadeToBlackBy(leds, NUM_LEDS, 6);
  static unsigned long nextUpdate[1]  ;
  static uint16_t segmentOffset[1] ; 
  PatternRotateThoughSubSections( nextUpdate[0], segmentOffset[0], PATTERN_TRIANGLES , ARRAY_SIZE(PATTERN_TRIANGLES ), 12, 700, 1, gHue);
}

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
*/
void Sixth() {  
  fadeToBlackBy(leds, NUM_LEDS, 6);
  static unsigned long nextUpdate[1];
  static uint16_t segmentOffset[1]; 
  PatternRotateThoughSubSections( nextUpdate[0], segmentOffset[0], PATTERN_SIXTH, ARRAY_SIZE(PATTERN_SIXTH), 7, 200, 1, gHue);   
}

void NorthenLights()
{
  fadeToBlackBy(leds, NUM_LEDS, 3);

  PatternSetColor( PATTERN_MOUNTAIN_ONE, ARRAY_SIZE(PATTERN_MOUNTAIN_ONE), gHue + (255/14) * 0) ;
  PatternSetColor( PATTERN_MOUNTAIN_TWO, ARRAY_SIZE(PATTERN_MOUNTAIN_TWO), gHue + (255/14) * 2) ;
  PatternSetColor( PATTERN_MOUNTAIN_THREE, ARRAY_SIZE(PATTERN_MOUNTAIN_THREE), gHue + (255/14) * 4) ;
  PatternSetColor( PATTERN_MOUNTAIN_FOUR, ARRAY_SIZE(PATTERN_MOUNTAIN_FOUR), gHue + (255/14) * 6) ;
  PatternSetColor( PATTERN_MOUNTAIN_FIVE, ARRAY_SIZE(PATTERN_MOUNTAIN_FIVE), gHue + (255/14) * 8) ;
  PatternSetColor( PATTERN_MOUNTAIN_SIX, ARRAY_SIZE(PATTERN_MOUNTAIN_SIX), gHue + (255/14) * 10) ;
  PatternSetColor( PATTERN_MOUNTAIN_SEVEN, ARRAY_SIZE(PATTERN_MOUNTAIN_SEVEN), gHue + (255/14) * 12) ;    

  EVERY_N_MILLISECONDS(50)
  {
      leds[PATTERN_SKY[random8(ARRAY_SIZE(PATTERN_SKY))]] = CHSV(HUE_GREEN + random8(HUE_BLUE - HUE_AQUA), 200, 255);
  }
}

void Lighting() {
  fadeToBlackBy(leds, NUM_LEDS, 10); 

  PatternSetColor( PATTERN_MOUNTAIN_ONE, ARRAY_SIZE(PATTERN_MOUNTAIN_ONE), gHue + (255/14) * 0) ;
  PatternSetColor( PATTERN_MOUNTAIN_TWO, ARRAY_SIZE(PATTERN_MOUNTAIN_TWO), gHue + (255/14) * 2) ;
  PatternSetColor( PATTERN_MOUNTAIN_THREE, ARRAY_SIZE(PATTERN_MOUNTAIN_THREE), gHue + (255/14) * 4) ;
  PatternSetColor( PATTERN_MOUNTAIN_FOUR, ARRAY_SIZE(PATTERN_MOUNTAIN_FOUR), gHue + (255/14) * 6) ;
  PatternSetColor( PATTERN_MOUNTAIN_FIVE, ARRAY_SIZE(PATTERN_MOUNTAIN_FIVE), gHue + (255/14) * 8) ;
  PatternSetColor( PATTERN_MOUNTAIN_SIX, ARRAY_SIZE(PATTERN_MOUNTAIN_SIX), gHue + (255/14) * 10) ;
  PatternSetColor( PATTERN_MOUNTAIN_SEVEN, ARRAY_SIZE(PATTERN_MOUNTAIN_SEVEN), gHue + (255/14) * 12) ;    


  const uint8_t size = 1 ; 
  uint8_t lightingOffset = random8(ARRAY_SIZE(PATTERN_SKY)-size);
  EVERY_N_MILLISECONDS(20)
  {
    PatternSetColor( PATTERN_SKY + lightingOffset, size, HUE_YELLOW ) ;
  }


}