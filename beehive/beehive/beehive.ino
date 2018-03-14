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
#define NUM_LEDS 38
CRGB leds[NUM_LEDS];

#define BRIGHTNESS 200
#define FRAMES_PER_SECOND 120

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

const unsigned char PATTERN_FLOWER[] = { 4, 9, 14, 19, 24, 29 };
const unsigned char PATTERN_HEX[] = { 0, 3, 5, 8, 10, 13, 15, 18, 20, 23, 25, 28 };
const unsigned char PATTERN_CORNER[] = { 1, 2, 6, 7, 11, 12, 16, 17, 21, 22, 26, 27 };

const unsigned char COUNT_FLOWERS = 6;
const unsigned char COUNT_HEXS = 12;
const unsigned char COUNT_CORNER = 12;

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
SimplePatternList gPatterns = { SpiralInwards, RandomGlow, RotatingSplitHalf, RotatingSplitThirds, rainbow, Flower, WalkPatterns, sinelon, juggle, /*Snake,*/ bpm };

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

void HelperCorner(unsigned char hue, unsigned char offset)
{
    leds[PATTERN_CORNER[offset % COUNT_CORNER]] = CHSV(hue, 200, 255);
}
void HelperHex(unsigned char hue, unsigned char offset)
{
    leds[PATTERN_HEX[offset % COUNT_HEXS]] = CHSV(hue, 200, 255);
}
void HelperFlower(unsigned char hue, unsigned char offset)
{
    leds[PATTERN_FLOWER[offset % COUNT_FLOWERS]] = CHSV(hue, 200, 255);
}

void HelperFillCorner(unsigned char hue)
{
    for (unsigned short offset = 0; offset < COUNT_CORNER; offset++) {
        leds[PATTERN_CORNER[offset]] = CHSV(hue, 200, 255);
    }
}
void HelperFillHex(unsigned char hue)
{
    for (unsigned short offset = 0; offset < COUNT_HEXS; offset++) {
        leds[PATTERN_HEX[offset]] = CHSV(hue, 200, 255);
    }
}
void HelperFillFlowers(unsigned char hue)
{
    for (unsigned short offset = 0; offset < COUNT_FLOWERS; offset++) {
        leds[PATTERN_FLOWER[offset]] = CHSV(hue, 200, 255);
    }
}

// ----------------------------------------------------------------------------

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

void Flower()
{
    static const unsigned char BLARG = 30;
    static const unsigned char LAYER_OFFSET = 255 / 3;

    for (unsigned short offset = 0; offset < COUNT_FLOWERS; offset++) {
        leds[PATTERN_FLOWER[offset]] = CHSV(gHue + (LAYER_OFFSET * 0) + (BLARG / COUNT_FLOWERS) * offset, 200, 255);
    }

    for (unsigned short offset = 0; offset < COUNT_HEXS; offset++) {
        leds[PATTERN_HEX[offset]] = CHSV(gHue + (LAYER_OFFSET * 1) + (BLARG / COUNT_FLOWERS) * offset, 200, 255);
    }

    for (unsigned short offset = 0; offset < COUNT_CORNER; offset++) {
        leds[PATTERN_CORNER[offset]] = CHSV(gHue + (LAYER_OFFSET * 2) + (BLARG / COUNT_FLOWERS) * offset, 200, 255);
    }

    gHue += 2;
}

void WalkPatterns()
{
    fadeToBlackBy(leds, NUM_LEDS, 10);

    const unsigned short SPEED = 2000;

    // HEXS
    // ============================================================
    static unsigned short offsetHex = 0;
    if (offsetHex >= COUNT_HEXS) {
        offsetHex = 0;
    }
    HelperHex(gHue, offsetHex);

    static unsigned long nextUpdatedHex = 0;
    if (nextUpdatedHex < millis()) {
        nextUpdatedHex = millis() + SPEED / COUNT_HEXS;
        offsetHex++;
    }

    // Flower
    // ============================================================
    static short offsetFlower = COUNT_FLOWERS - 1;
    if (offsetFlower < 0) {
        offsetFlower = COUNT_FLOWERS - 1;
    }
    static const unsigned char BLARG = 10;
    HelperFlower(gHue + 128 + (BLARG / COUNT_FLOWERS) * offsetFlower, offsetFlower);

    static unsigned long nextUpdatedFlower = 0;
    if (nextUpdatedFlower < millis()) {
        nextUpdatedFlower = millis() + SPEED / COUNT_FLOWERS;
        offsetFlower--;
    }

    // corner
    // ============================================================
    static short offsetCorner = 0;
    if (offsetCorner >= COUNT_CORNER) {
        offsetCorner = 0;
    }
    HelperCorner(gHue + 128, offsetCorner);

    static unsigned long nextUpdatedCorner = 0;
    if (nextUpdatedCorner < millis()) {
        nextUpdatedCorner = millis() + SPEED / COUNT_CORNER;
        offsetCorner++;
    }
}

void Snake()
{
    fadeToBlackBy(leds, NUM_LEDS, 10);

    const unsigned short SPEED = 1000;
    static unsigned short offset = 0;

    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED / NUM_LEDS;
        offset++;
    }

    if (offset > NUM_LEDS) {
        offset = 0;
    }    

    leds[offset] = CHSV(gHue, 200, 255);
}

void RotatingSplit(unsigned int splits)
{
    fadeToBlackBy(leds, NUM_LEDS, 10);

    static unsigned short offset = 0;
    const unsigned short SPEED = 2000;
    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED / NUM_LEDS;
        offset++;
    }

    const unsigned short sizeOfSlice = NUM_LEDS / splits;

    for (unsigned int section = 0; section < 6; section++) {
        for (unsigned int n = 0; n < sizeOfSlice; n++) {
            unsigned int z = section * sizeOfSlice + n + offset;

            leds[z % NUM_LEDS] = CHSV(gHue + (255 / 3) * section, 200, 255);
        }
    }
}

void RotatingSplitThirds()
{
    RotatingSplit(3);
}
void RotatingSplitHalf()
{
    RotatingSplit(2);
}

void RandomGlow()
{
    const unsigned short SPEED = 100;
    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED;

        static unsigned char hue = 0;

        leds[random16(NUM_LEDS)] = CHSV(hue, 200, 255);
        hue++;
    }

    static unsigned long nextUpdatedFade = 0;
    if (nextUpdatedFade < millis()) {
        nextUpdatedFade = millis() + 5;

        fadeToBlackBy(leds, NUM_LEDS, 1);
    }
}

void SpiralInwards()
{
    static unsigned long nextUpdatedFade = 0;
    if (nextUpdatedFade < millis()) {
        nextUpdatedFade = millis() + 5;

        fadeToBlackBy(leds, NUM_LEDS, 1);
    }

    const unsigned short SPEED = 500;
    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED;

        static unsigned char hue = 0;
        hue += 15;

        static unsigned char type = 0;
        if (type > 2) {
            type = 0;
        }

        switch (type) {
            case 0: {
                HelperFillCorner(hue);
                break;
            }
            case 1: {
                HelperFillHex(hue);
                break;
            }
            case 2: {
                HelperFillFlowers(hue);
                break;
            }
        }

        type++;
    }
}
