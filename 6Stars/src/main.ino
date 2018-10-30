/**
 * 6Star - Stainglass window 
 * Written by: Steven Smethurst 
 * More info https://blog.abluestar.com/projects/2018-stained-glass-window 
 */ 
const char APP_NAME[] = "6Star";
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
#define NUM_LEDS    30
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// Settings 
const uint16_t SETTING_TIME_FOR_EACH_PATTERN = 30 ; 

// Patterns 
const uint8_t STARS[] = {4,8,18,25,21,11};
const uint8_t EDGES[] = {1,0,6,7,17,27,28,29,23,22,12,2};
const uint8_t HEXES[] = {3,5,10,13,15,16,20,24,26};
const uint8_t CENTERS[] = {9,14,19};

const uint8_t PATTERN_TRIANGLES[] = {9,5,6,7,8, 15,16,17,18,27,  19,26,28,25,29,  20,24,23,21,22,  12,13,14,11,2, 1,0,3,4,10 };

#define PATTERN_RINGS_ONE EDGES
const uint8_t PATTERN_RINGS_TWO[] = {3,4,5,8,16,18,26,25,24,21,13,11};
const uint8_t PATTERN_RINGS_THREE[] = {9,15,19,20,14,10};

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
SimplePatternList gPatterns = { Rings, Clock, Triangles, Groups, confetti, rainbow, sinelon, bpm, juggle };

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
/*
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
}

void LargeTriangles () {
  fadeToBlackBy(leds, NUM_LEDS, 4);
  static unsigned long nextUpdate = 0 ;
  static uint16_t segmentOffset = 0 ; 
  PatternRotateThoughSubSections( nextUpdate, segmentOffset, PATTERN_LARGE_TRIANGLES, ARRAY_SIZE(PATTERN_LARGE_TRIANGLES), 9, 500, 1, gHue+ 127 * segmentOffset);   
}
*/
void Groups() {
  PatternSetColor( STARS, ARRAY_SIZE(STARS), gHue + (255/4) * 0) ;
  PatternSetColor( EDGES, ARRAY_SIZE(EDGES), gHue + (255/4) * 1) ;
  PatternSetColor( HEXES, ARRAY_SIZE(HEXES), gHue + (255/4) * 2) ;
  PatternSetColor( CENTERS, ARRAY_SIZE(CENTERS), gHue + (255/4) * 3) ;
}

void Triangles() {
  fadeToBlackBy(leds, NUM_LEDS, 4);
  static unsigned long nextUpdate = 0 ;
  static uint16_t segmentOffset = 0 ; 
  PatternRotateThoughSubSections( nextUpdate, segmentOffset, PATTERN_TRIANGLES , ARRAY_SIZE(PATTERN_TRIANGLES ), 5, 100, 1, gHue);   
}

void Clock() {
  fadeToBlackBy(leds, NUM_LEDS, 4);

  static unsigned long edgesNextUpdate = 0 ;
  static uint16_t edgesSegmentOffset = 0 ; 
  PatternRotateThoughSubSections( edgesNextUpdate, edgesSegmentOffset, EDGES , ARRAY_SIZE(EDGES), 1, 150, 1, gHue);   

  static unsigned long starsNextUpdate = 0 ;
  static uint16_t starsSegmentOffset = 0 ; 
  PatternRotateThoughSubSections( starsNextUpdate, starsSegmentOffset, STARS , ARRAY_SIZE(STARS), 1, 150, -1, gHue + ((255/3) * 1));   

  static unsigned long centersNextUpdate = 0 ;
  static uint16_t centersSegmentOffset = 0 ; 
  PatternRotateThoughSubSections( centersNextUpdate, centersSegmentOffset, CENTERS , ARRAY_SIZE(CENTERS), 1, 300, 1, gHue + ((255/3) * 2));   

  PatternSetColor( HEXES, ARRAY_SIZE(HEXES), gHue + (255/3) * 3) ;
}

void Rings() {
  PatternSetColor( PATTERN_RINGS_ONE, ARRAY_SIZE(PATTERN_RINGS_ONE), gHue + (255/3) * 0);
  PatternSetColor( PATTERN_RINGS_TWO, ARRAY_SIZE(PATTERN_RINGS_TWO), gHue + (255/3) * 1);
  PatternSetColor( PATTERN_RINGS_THREE, ARRAY_SIZE(PATTERN_RINGS_THREE), gHue + (255/3) * 2);
}
