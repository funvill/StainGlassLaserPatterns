#include "FastLED.h"
FASTLED_USING_NAMESPACE

#define DATA_PIN    D6
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB

const uint8_t NUM_LEDS = 25;
const uint8_t FRAMES_PER_SECOND = 120;

// Settings 
const uint8_t SETTING_BRIGHTNESS = 200;

// Patterns 
const unsigned char PATTERN_SUN[] = { 0,1,2,3,4,5,6,7,8 };
const unsigned char PATTERN_RAYS[] = { 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24 };
const unsigned char PATTERN_SUN_LED_COUNT = 9 ;
const unsigned char PATTERN_RAY_LED_COUNT = NUM_LEDS - PATTERN_SUN_LED_COUNT ;
const unsigned char PATTERN_RAY_COUNT = 11 ; 
const unsigned char PATTERN_SUNBARS_COUNT = 5 ; 



CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;



void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(SETTING_BRIGHTNESS);

  fill_solid( currentPalette, 16, CRGB::Red);
  currentPalette[0] = CRGB::DarkRed;
  currentPalette[4] = CRGB::Maroon;
  currentPalette[6] = CRGB::Orange; 
  currentPalette[8] = CRGB::Yellow;
  currentPalette[10] = CRGB::Gold;
  currentPalette[12] = CRGB::Goldenrod;
  currentPalette[13] = CRGB::White;
  currentPalette[14] = CRGB::White;
  currentPalette[15] = CRGB::White;

}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { BlueRedRays, SunRainbow,  confetti, sinelon, rainbowWithGlitter, juggle, bpm };

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
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void HelperSetSunBars( unsigned char offset, CRGB value ) {
  switch(offset % 5 ) {
    case 0:
      leds[ 0 ] =  value; 
      leds[ 1 ] =  value; 
      break; 
    case 1:
      leds[ 2 ] =  value; 
      leds[ 3 ] =  value; 
      break; 
    case 2:
      leds[ 4 ] =  value; 
      leds[ 5 ] =  value; 
      break; 
    case 3:
      leds[ 6 ] =  value; 
      leds[ 7 ] =  value; 
      break; 
    case 4:
      leds[ 8 ] =  value; 
      break; 
  }
}

void HelperSetRays( unsigned char offset, CRGB value ) {
  switch( offset % PATTERN_RAY_COUNT) {
    case 0: 
      leds[ 9 ] =  value; 
      break ; 
    case 1: 
      leds[ 10 ] =  value; 
      break ; 
    case 2: 
      leds[ 11 ] =  value; 
      break ; 
    case 3: 
      leds[ 12 ] =  value; 
      leds[ 13 ] =  value; 
      break ; 
    case 4: 
      leds[ 14 ] =  value; 
      leds[ 15 ] =  value; 
      break ; 
    case 5: 
      leds[ 16 ] =  value; 
      leds[ 17 ] =  value; 
      break ; 
    case 6: 
      leds[ 18 ] =  value; 
      leds[ 19 ] =  value; 
      break ; 
    case 7: 
      leds[ 20 ] =  value; 
      leds[ 21 ] =  value; 
      break ;
    case 8: 
      leds[ 22 ] =  value; 
      break ; 
    case 9: 
      leds[ 23 ] =  value; 
      break ; 
    case 10: 
      leds[ 24 ] =  value; 
      break ; 
  }
}



void SunRainbow() {

  const unsigned short SPEED = 50;
  static unsigned long nextUpdated = 0;
  static unsigned char sunOffset = 0 ; 
  if (nextUpdated < millis()) {
    nextUpdated = millis() + SPEED;
    sunOffset++; 
  }
  
  for( int offset = 0 ; offset < PATTERN_SUN_LED_COUNT ; offset++ ) {    
    leds[ offset ] =  ColorFromPalette( currentPalette, sunOffset + offset, 255, LINEARBLEND);
  }

  fill_rainbow( leds + PATTERN_SUN_LED_COUNT, PATTERN_RAY_LED_COUNT, gHue, 7);
}


void BlueRedRays() {
  const unsigned short SPEED = 200;
  static unsigned long nextUpdated = 0;
  static unsigned char rayOffset = 0 ; 
  if (nextUpdated < millis()) {
    nextUpdated = millis() + SPEED;
    rayOffset++; 
  }

  unsigned char i = 0 ; 
  for( unsigned int offset = 0 ; offset < PATTERN_RAY_COUNT ; offset ++ ) {
    if( (offset + rayOffset ) % 2 == 0 ) {      
      HelperSetRays(offset, CRGB::Red );     
    } else {
      HelperSetRays(offset, CRGB::Blue );
    }
  }

  for( unsigned int offset = 0 ; offset < PATTERN_SUNBARS_COUNT  ; offset ++ ) {
    HelperSetSunBars(offset, CHSV( gHue + (255/(PATTERN_SUNBARS_COUNT*2)) * offset, 255, 255) ); 
  }
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

