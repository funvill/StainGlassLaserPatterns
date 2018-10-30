/**
 * Simple Triangles - Stainglass window 
 * Written by: Steven Smethurst 
 * More info https://blog.abluestar.com/projects/2018-stained-glass-window 
 */ 
const char APP_NAME[] = "Simple Triangles";
const uint16_t APP_VERSION_MAJOR = 0;
const uint16_t APP_VERSION_MINOR = 0;
const uint16_t APP_VERSION_PATCH = 3;

#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    D6
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
#define NUM_LEDS    24
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// Settings 
const uint16_t SETTING_TIME_FOR_EACH_PATTERN = 30 ; 

// Patterns 
const uint8_t PATTERN_GEMS[] = {0,1,2,8,9,10,  2,3,4,6,7,8,  7,6,5,18,17,16,  15,16,17,19,20,21,  13,14,15,21,22,23,  11,10,9,14,13,12,  9,8,7,16,15,14}; 
const uint8_t PATTERN_SPIRAL[] = {0,1,2,3,4,6,5,18,17,19,20,21,22,23,13,12,11,10,9,8,7,16,15,14};
const uint8_t PATTERN_RINGS_OUTSIDE[] = {0,1,2,3,4,6,5,18,17,19,20,21,22,23,13,12,11,10};
const uint8_t PATTERN_RINGS_INSIDE[] = {9,8,7,16,15,14};
const uint8_t PATTERN_STAR[] = {10,2,6,17,21,13,9,8,7,16,15,14};

const uint8_t PATTERN_GROUPS_POINTING_INWARDS[] = {0,1,3,4,5,18,19,20,22,23,12,11};
const uint8_t PATTERN_GROUPS_POINTING_OUTWARDS[] = {2,6,17,21,13,10};
const uint8_t PATTERN_GROUPS_CENTER[] = {8,16,14, 7,15,9};

const uint8_t PATTERN_RHOMBUS[] = {0,1,  2,3,  4,6,  5,18,  17,19,  20,21,  22,23,  13,12,  11,10,  9,8,  7,16,  15,14 };

const uint8_t PATTERN_MEDIUM_TRIANGLES[] = {11,9,0,10, 1,8,3,2,  4,7,5,6, 18,16,19,17, 20,15,22,21, 23,14,12,13 };
const uint8_t PATTERN_LARGE_TRIANGLES[] = { 10,8,6,9,7,14,15,16,21,  2,8,9,7,13,14,15,16,17 };



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
SimplePatternList gPatterns = { LargeTriangles, MediumTriangles, Rhombus, Groups, StarRainbow, confetti, Gems, rainbow, SpiralInwards, sinelon, Rings, bpm, juggle };

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

void StarRainbow() {
  fadeToBlackBy(leds, NUM_LEDS, 5);
  PatternSetColor( PATTERN_STAR, ARRAY_SIZE(PATTERN_STAR), gHue ) ;
}

void Gems() {
  fadeToBlackBy(leds, NUM_LEDS, 1);

  static unsigned long nextUpdate = 0 ;
  static uint16_t segmentOffset = 0 ; 
  // -6 so we don't do the center gem. 
  PatternRotateThoughSubSections( nextUpdate, segmentOffset, PATTERN_GEMS, ARRAY_SIZE(PATTERN_GEMS) - 6, 6, 300, -1, gHue); 
}

void SpiralInwards() {  
  fadeToBlackBy(leds, NUM_LEDS, 1);
  static unsigned long nextUpdate = 0 ;
  static uint16_t segmentOffset = 0 ; 
  PatternRotateThoughSubSections( nextUpdate, segmentOffset, PATTERN_SPIRAL, ARRAY_SIZE(PATTERN_SPIRAL) , 1, 20, 1, gHue);   
}

void Rings() {
  fadeToBlackBy(leds, NUM_LEDS, 5);

  static unsigned long insideNextUpdate = 0 ;
  static uint16_t insideSegmentOffset = 0 ; 
  static unsigned long outsideNextUpdate = 0 ;
  static uint16_t outsideSegmentOffset = 0 ; 

  PatternRotateThoughSubSections( insideNextUpdate, insideSegmentOffset, PATTERN_RINGS_INSIDE, ARRAY_SIZE(PATTERN_RINGS_INSIDE), 1, 100, 1, gHue);   
  PatternRotateThoughSubSections( outsideNextUpdate, outsideSegmentOffset, PATTERN_RINGS_OUTSIDE, ARRAY_SIZE(PATTERN_RINGS_OUTSIDE), 1, 20, -1, gHue + 127 );   
}

void MediumTriangles () {
  PatternSegmentRainbow(PATTERN_MEDIUM_TRIANGLES, ARRAY_SIZE(PATTERN_MEDIUM_TRIANGLES), 4, gHue) ;
/*
  uint16_t segmentsSize = 4 ; 
  uint16_t segmentsCount = ARRAY_SIZE(PATTERN_MEDIUM_TRIANGLES) / segmentsSize ; 
  for( uint16_t segmentOffset = 0 ; segmentOffset < segmentsCount ; segmentOffset++) {
    PatternSetColor( PATTERN_MEDIUM_TRIANGLES + ( segmentOffset * segmentsSize), segmentsSize, gHue + (255 / segmentsCount) * segmentOffset) ;
  }
  */
}

void LargeTriangles () {
  fadeToBlackBy(leds, NUM_LEDS, 4);
  static unsigned long nextUpdate = 0 ;
  static uint16_t segmentOffset = 0 ; 
  PatternRotateThoughSubSections( nextUpdate, segmentOffset, PATTERN_LARGE_TRIANGLES, ARRAY_SIZE(PATTERN_LARGE_TRIANGLES), 9, 500, 1, gHue+ 127 * segmentOffset);   
}

void Groups() {

  PatternSetColor( PATTERN_GROUPS_POINTING_INWARDS, ARRAY_SIZE(PATTERN_GROUPS_POINTING_INWARDS), gHue + (255/4) * 0) ;
  PatternSetColor( PATTERN_GROUPS_POINTING_OUTWARDS, ARRAY_SIZE(PATTERN_GROUPS_POINTING_OUTWARDS), gHue + (255/4) * 1) ;

  PatternSetColor( PATTERN_GROUPS_CENTER, ARRAY_SIZE(PATTERN_GROUPS_CENTER) - 3 , gHue + (255/4) * 2) ;
  PatternSetColor( PATTERN_GROUPS_CENTER + (sizeof(char) * 3), ARRAY_SIZE(PATTERN_GROUPS_CENTER) - 3 , gHue + (255/4) * 3) ;
}

void Rhombus() {
  fadeToBlackBy(leds, NUM_LEDS, 10);

  static uint16 SPEED = 350; 
  static long nextUpdate = 0 ; 
  // Only update the segments on a timer. 
  if (nextUpdate < millis()) {
    nextUpdate = millis() + SPEED;
    SPEED += 20 ; 

    uint16_t segmentsCount = ARRAY_SIZE(PATTERN_RHOMBUS) / 2 ; 
    for( uint16_t segmentOffset = 0 ; segmentOffset < segmentsCount ; segmentOffset++) {
      PatternSetColor( PATTERN_RHOMBUS + ( segmentOffset * 2), 2, (255 / segmentsCount) * random(0, segmentsCount)) ;
    }
  }
}
