/**
 * Heart - Stainglass window 
 * Written by: Steven Smethurst 
 * More info https://blog.abluestar.com/projects/2018-stained-glass-window 
 */ 
#define FASTLED_ALLOW_INTERRUPTS 0
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

#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120

const unsigned char PATTERN_GEMS_BOTTOM_RIGHT[] = {4,5,6,7,8,9}; 
const unsigned char PATTERN_GEMS_BOTTOM_LEFT[] = {38,39,40,41,42,43};
const unsigned char PATTERN_GEMS_TOP_RIGHT[] = {16,17,18,19,20,21};
const unsigned char PATTERN_GEMS_TOP_LEFT[] =  {26,27,28,29,30,31}; 

const unsigned char PATTERN_OUTSIDE[] = {0,12,23,35,47};
const unsigned char PATTERN_HEART_WHOLE[] = {1,2,3,4,5,6,7,8,9,10,11, 13,14,15,16,17,18,19,20,21,22,  24,25,26,27,28,29,30,31,32,33,34,  36,37,38,39,40,41,42,43,44,45,46 }; 
const unsigned char PATTERN_HEART_CENTER[] = { 6,8,18,19,28,29,39,41}; 
const unsigned char PATTERN_HEART_OUTSIDE[] = { 1,2,3,4,5,7,9,10,11,13,14,15,16,17,20,21,22,24,25,26,27,30,31,32,33,34,36,37,38,40,42,43,44,45,46}; 

void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

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
SimplePatternList gPatterns = {  RainbowWholeHeart, HeartBeat, InsideHeart, Gems/*,  rainbow, Rings, Clock, PatternConer*/, confetti, bpm };

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
  EVERY_N_SECONDS( 30 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

/*
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
*/

void HeartBeat() {
  
  // boarder 
  // fadeToBlackBy(leds, NUM_LEDS, 1);

  //                                          M    H    L    H     L 
  const unsigned short HEART_BEAT_TIMES[] = { 30, 100, 200, 110,  30, 1200};
  const unsigned char HEART_BEAT_LEVELS[] = {100, 125, 80,  200, 125,   75};

  static unsigned char heartBeatTimerIndex = 0 ; 
  static long lastUpdate = 0 ; 

  if( millis() - lastUpdate > HEART_BEAT_TIMES[heartBeatTimerIndex] ) {
    heartBeatTimerIndex++ ; 
    if(heartBeatTimerIndex >= sizeof(HEART_BEAT_LEVELS) ) {
      // Loop 
      heartBeatTimerIndex = 0 ; 
    }
    // Update the timer. 
    lastUpdate = millis() ; 
  }

  for( unsigned char offset = 0 ; offset < sizeof(PATTERN_HEART_WHOLE) ; offset++ ) {
    leds[PATTERN_HEART_WHOLE[offset]] = CHSV( 15, 255, HEART_BEAT_LEVELS[heartBeatTimerIndex]);
  }

  // outside (nice blue) 
  for( unsigned char offset = 0 ; offset < sizeof(PATTERN_OUTSIDE) ; offset++ ) {
    leds[PATTERN_OUTSIDE[offset]] = CRGB( 0, 0, 150); 
  }  
}

void RainbowWholeHeart() {

  for( unsigned char offset = 0 ; offset < sizeof(PATTERN_HEART_WHOLE) ; offset++ ) {
    leds[PATTERN_HEART_WHOLE[offset]] = CHSV(gHue + (offset * 30), 200, 255);  
  } 

  for( unsigned char offset = 0 ; offset < sizeof(PATTERN_OUTSIDE) ; offset++ ) {
    leds[PATTERN_OUTSIDE[offset]] = CHSV(gHue, 120, 255);  
  }   
  
}

void InsideHeart() {

  // boarder 
  fadeToBlackBy(leds, NUM_LEDS, 5);

  // Small heart 
  for( unsigned char offset = 0 ; offset < sizeof(PATTERN_HEART_CENTER) ; offset++ ) {
    leds[PATTERN_HEART_CENTER[offset]] = CHSV(gHue, 200, 255);  
  }  
  // Large heart 
  for( unsigned char offset = 0 ; offset < sizeof(PATTERN_HEART_OUTSIDE) ; offset++ ) {
    leds[PATTERN_HEART_OUTSIDE[offset]] = CHSV(gHue + (255/2), 200, 255);  
  }
}

void Gems() {

  // boarder 
  fadeToBlackBy(leds, NUM_LEDS, 5);
  
  // Inside heart 
  for( unsigned char offset = 0 ; offset < sizeof(PATTERN_HEART_WHOLE) ; offset++ ) {
    leds[PATTERN_HEART_WHOLE[offset]] = CHSV(gHue + 30, 60, 255);  
  } 

  // Gems 
  for( unsigned char offset = 0 ; offset < sizeof(PATTERN_GEMS_BOTTOM_RIGHT) ; offset++ ) {
    leds[PATTERN_GEMS_BOTTOM_RIGHT[offset]] = CHSV(gHue, 200, 255);  
  }

  for( unsigned char offset = 0 ; offset < sizeof(PATTERN_GEMS_BOTTOM_LEFT) ; offset++ ) {
    leds[PATTERN_GEMS_BOTTOM_LEFT[offset]] = CHSV(gHue + 120, 200, 255);  
  }

  for( unsigned char offset = 0 ; offset < sizeof(PATTERN_GEMS_TOP_RIGHT) ; offset++ ) {
    leds[PATTERN_GEMS_TOP_RIGHT[offset]] = CHSV(gHue + 180, 200, 255);  
  }
  
  for( unsigned char offset = 0 ; offset < sizeof(PATTERN_GEMS_TOP_LEFT) ; offset++ ) {
    leds[PATTERN_GEMS_TOP_LEFT[offset]] = CHSV(gHue + 60, 200, 255);  
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

