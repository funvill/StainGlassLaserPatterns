#include "FastLED.h"

FASTLED_USING_NAMESPACE

// https://github.com/FastLED/FastLED/wiki/ESP8266-notes
#define FASTLED_ESP8266_D1_PIN_ORDER

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

#define DATA_PIN D6
#define LED_TYPE WS2812
#define COLOR_ORDER RGB
#define NUM_LEDS 31

CRGB leds[NUM_LEDS];

#define BRIGHTNESS 200
#define FRAMES_PER_SECOND 120

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

const unsigned char COUNT_OFFSET = 1;
const unsigned char COUNT_SKY = 16;
const unsigned char COUNT_MOUNTAIN = 14;

void setup()
{
    delay(3000); // 3 second delay for recovery
    Serial.begin(9600);

    // tell FastLED about the LED strip configuration
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

    // set master brightness control
    FastLED.setBrightness(BRIGHTNESS);

    fadeToBlackBy(leds, NUM_LEDS, 20);
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbowMounts, northenLights };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

void loop()
{

    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000 / FRAMES_PER_SECOND);

    // do some periodic updates
    EVERY_N_MILLISECONDS(20) { gHue++; } // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS(20) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);

    Serial.println(gCurrentPatternNumber);
}

// ----------------------------------------------------------------------------

void HelperSetMountain(unsigned char offset, unsigned char hue) {

    switch(offset) {
        default: 
        case 0:
        {
            for (unsigned short offset = 0; offset < 3; offset++) {
                leds[COUNT_OFFSET + offset] = CHSV(hue, 200, 255);
            }  
            break ; 
        }
        case 1:
        {
            for (unsigned short offset = 3; offset < 4; offset++) {
                leds[COUNT_OFFSET + offset] = CHSV(hue, 200, 255);
            }  
            break ; 
        }
        case 2:
        {
            for (unsigned short offset = 4; offset < 6; offset++) {
                leds[COUNT_OFFSET + offset] = CHSV(hue, 200, 255);
            }  
            break ; 
        }
        case 3:
        {
            for (unsigned short offset = 6; offset < 9; offset++) {
                leds[COUNT_OFFSET + offset] = CHSV(hue, 200, 255);
            }  
            break ; 
        }
        case 4:
        {
            for (unsigned short offset = 9; offset < 11; offset++) {
                leds[COUNT_OFFSET + offset] = CHSV(hue, 200, 255);
            }  
            break ; 
        }
        case 5:
        {
            for (unsigned short offset = 11; offset < 12; offset++) {
                leds[COUNT_OFFSET + offset] = CHSV(hue, 200, 255);
            }  
            break ; 
        }
        case 6:
        {
            for (unsigned short offset = 12; offset < 14; offset++) {
                leds[COUNT_OFFSET + offset] = CHSV(hue, 200, 255);
            }  
            break ; 
        }
    }
}

void HelperFillMountains(unsigned char hue)
{
    for (unsigned short offset = 0; offset < COUNT_MOUNTAIN; offset++) {
        leds[COUNT_OFFSET + offset] = CHSV(hue, 200, 255);
    }
}

void HelperFillSky(unsigned char hue)
{
    for (unsigned short offset = 0; offset < COUNT_SKY; offset++) {
        leds[COUNT_OFFSET + COUNT_MOUNTAIN + offset] = CHSV(hue, 200, 255);
    }
}

// ----------------------------------------------------------------------------

void rainbowMounts() {

    static unsigned char skyHue = 0 ;
    static unsigned char mountainHue = 0 ; 
    static unsigned char mountainOffset = 0 ; 
    if( mountainOffset > 6) {
        mountainOffset = 0 ; 
    }

    HelperSetMountain(mountainOffset, mountainHue) ; 

    fadeToBlackBy(leds, NUM_LEDS, 5);
    EVERY_N_MILLISECONDS(50)
    {
        mountainOffset = random8(7);
        mountainHue += 32 ; 
        skyHue++;

        leds[COUNT_OFFSET + COUNT_MOUNTAIN + random8(COUNT_SKY)] = CHSV(skyHue, 200, 255);
        leds[COUNT_OFFSET + COUNT_MOUNTAIN + random8(COUNT_SKY)] = CHSV(skyHue+255/2, 200, 255);
    }    
}

void northenLights()
{
    static unsigned char hue = 0 ;     
    HelperFillMountains( hue );

    fadeToBlackBy(leds, NUM_LEDS, 7);

    EVERY_N_MILLISECONDS(50)
    {
        hue++; 
        leds[COUNT_OFFSET + COUNT_MOUNTAIN + random8(COUNT_SKY)] = CHSV(HUE_GREEN + random8(HUE_BLUE - HUE_AQUA), 200, 255);
    }
}

// ----------------------------------------------------------------------------
/*
void rainbow()
{
    // FastLED's built-in rainbow generator
    fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter)
{
    if (random8() < chanceOfGlitter) {
        leds[random16(NUM_LEDS)] += CRGB::White;
    }
}

void confetti()
{
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy(leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(leds, NUM_LEDS, 20);
    int pos = beatsin16(13, 0, NUM_LEDS);
    leds[pos] += CHSV(gHue, 255, 192);
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for (int i = 0; i < NUM_LEDS; i++) { //9948
        leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
}

void juggle()
{
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, NUM_LEDS, 20);
    byte dothue = 0;
    for (int i = 0; i < 8; i++) {
        leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}

*/