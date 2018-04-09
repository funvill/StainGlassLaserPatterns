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
#define NUM_LEDS 42
CRGB leds[NUM_LEDS];

#define BRIGHTNESS 200
#define FRAMES_PER_SECOND 120

// The status LED not part of the patterns.
// --------------------------------------------

// Stars
// Set of 5 leds per pettles. Starting bottom left going counter clockwise.
const unsigned char PATTERN_STARS[] = { 3, 4, 9, 14, 13, 10, 5, 6, 8, 16, 15, 9, 16, 17, 19, 29, 28, 20, 27, 28, 31, 38, 37, 32, 25, 26, 32, 36, 35, 33, 12, 13, 21, 25, 24, 22, 14, 15, 20, 27, 26, 21 };

// Squares
// Sets of 3 leds. Starting on the bottom. Counter clockwise.
const unsigned char PATTERN_SQUARES[] = { 2, 3, 10, 4, 5, 9, 6, 7, 8, 17, 18, 19, 15, 16, 20, 13, 14, 21, 11, 12, 22, 24, 25, 33, 26, 27, 32, 28, 29, 31, 37, 38, 40, 35, 36, 41 };

// Edge
// Sets OF 1
const unsigned char PATTERN_EDGES[] = { 1, 0, 7, 18, 30, 39, 40, 41, 34, 23, 11, 2 };

// Bars (14)
// Set 2, 3, 4, 3, 2
const unsigned char PATTERN_BARS[] = {

    1, 0, 10, 9, 8, 22, 21, 20, 19, 33, 32, 31, 41, 40,
    7, 18, 5, 16, 29, 3, 14, 27, 38, 12, 25, 36, 23, 34,
    30, 39, 17, 28, 37, 6, 15, 26, 35, 4, 13, 24, 2, 11
};

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

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
SimplePatternList gPatterns = { Snake, Bars, RoatingStar, Center, /*FlashStar, */ HueSquares, rainbow };

// Circles ,CycleInnward, Snake, RotatingPie, TypeHueRotate, RoatingBars, RotatingPie, RandomGlow, RandomPie, rainbow, sinelon, juggle, bpm

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
    EVERY_N_SECONDS(15) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);

    Serial.println(gCurrentPatternNumber);
}

// ----------------------------------------------------------------------------

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

void HelperFillCube(const unsigned char squareOffset, const unsigned char hue)
{
    if (squareOffset > ARRAY_SIZE(PATTERN_SQUARES) / 3) {
        return;
    }

    for (unsigned int offset = squareOffset * 3; offset < (squareOffset * 3) + 3; offset++) {
        leds[PATTERN_SQUARES[offset]] = CHSV(hue, 200, 255);
    }
}
void HelperFillStars(const unsigned char starOffset, const unsigned char hue)
{
    static const unsigned char COUNT_STAR = 6;
    if (starOffset > ARRAY_SIZE(PATTERN_STARS) / COUNT_STAR) {
        return;
    }

    for (unsigned int offset = starOffset * COUNT_STAR; offset < (starOffset * COUNT_STAR) + COUNT_STAR; offset++) {
        leds[PATTERN_STARS[offset]] = CHSV(hue, 200, 255);
    }
}

void HelperFillEdge(const unsigned char hue)
{
    for (unsigned char offset = 0; offset > ARRAY_SIZE(PATTERN_EDGES); offset++) {
        leds[PATTERN_EDGES[offset]] = CHSV(hue, 200, 255);
    }
}

void HueSquares()
{
    fadeToBlackBy(leds, NUM_LEDS, 1);

    unsigned char COUNT_CUBES = ARRAY_SIZE(PATTERN_SQUARES) / 3;
    for (unsigned char offset = 0; offset < COUNT_CUBES; offset++) {
        HelperFillCube(offset, gHue + ((255 / COUNT_CUBES / 2) * offset));
    }
}

void FlashStar()
{
    fadeToBlackBy(leds, NUM_LEDS, 1);

    const unsigned short SPEED = 500;
    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED;

        static const unsigned char COUNT_STARS = ARRAY_SIZE(PATTERN_STARS) / 6;
        static unsigned char lastOffset = 0;
        while (true) {
            unsigned char x = random8(COUNT_STARS);
            if (lastOffset != x) {
                lastOffset = x;
                break;
            }
        }
        HelperFillStars(random8(COUNT_STARS), gHue);
    }
}

void Center()
{
    static const unsigned char COUNT_SEGMENT = 5;

    // 0 - Center star
    HelperFillStars(6, gHue + ((255 / COUNT_SEGMENT) * 0));

    // 1 - Surounding star
    leds[9] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 1), 200, 255);
    leds[16] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 1), 200, 255);
    leds[28] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 1), 200, 255);
    leds[32] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 1), 200, 255);
    leds[25] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 1), 200, 255);
    leds[13] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 1), 200, 255);

    // 2 -
    leds[4] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 2), 200, 255);
    leds[5] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 2), 200, 255);
    leds[8] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 2), 200, 255);
    leds[17] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 2), 200, 255);
    leds[29] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 2), 200, 255);
    leds[31] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 2), 200, 255);
    leds[37] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 2), 200, 255);
    leds[36] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 2), 200, 255);
    leds[33] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 2), 200, 255);
    leds[24] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 2), 200, 255);
    leds[12] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 2), 200, 255);
    leds[10] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 2), 200, 255);

    // 3 -
    leds[3] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 3), 200, 255);
    leds[6] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 3), 200, 255);
    leds[19] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 3), 200, 255);
    leds[38] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 3), 200, 255);
    leds[35] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 3), 200, 255);
    leds[22] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 3), 200, 255);

    // 4 -
    leds[0] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 4), 200, 255);
    leds[1] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 4), 200, 255);
    leds[7] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 4), 200, 255);
    leds[18] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 4), 200, 255);
    leds[30] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 4), 200, 255);
    leds[39] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 4), 200, 255);
    leds[40] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 4), 200, 255);
    leds[41] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 4), 200, 255);
    leds[34] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 4), 200, 255);
    leds[23] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 4), 200, 255);
    leds[11] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 4), 200, 255);
    leds[2] = CHSV(gHue + ((255 / COUNT_SEGMENT) * 4), 200, 255);

    // HelperFillEdge( gHue + ((255 / COUNT_SEGMENT) * 4 ));
}

void RoatingStar()
{
    fadeToBlackBy(leds, NUM_LEDS, 5);

    const unsigned short SPEED = 200;
    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED;

        static unsigned char offset = 0;
        static const unsigned char COUNT_STARS = ARRAY_SIZE(PATTERN_STARS) / 6;

        HelperFillStars(offset % COUNT_STARS, gHue);
        offset++;
        if (offset % COUNT_STARS == COUNT_STARS - 1) {
            offset++;
        }
    }
}

void Bars()
{
    static const unsigned char COUNT_BAR_SIZE = 14;

    const unsigned short SPEED = 1000;
    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED;

        static unsigned char pieOffset = 0;
        if (pieOffset >= 3) {
            pieOffset = 0;
        }

        for (unsigned char offset = 0; offset < COUNT_BAR_SIZE; offset++) {
            leds[PATTERN_BARS[pieOffset * COUNT_BAR_SIZE + offset]] = CHSV(gHue, 200, 255);
        }

        pieOffset++;
    }
}

void Snake()
{
    static unsigned char offset = 0;

    const unsigned short SPEED = 100;
    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED;

        offset++;
        if (offset >= NUM_LEDS) {
            offset = 0;
        }
    }

    leds[offset] = CHSV(gHue, 200, 255);
    fadeToBlackBy(leds, NUM_LEDS, 5);
}

/*
void TypeHueRotate()
{
    HelperFillCorners(gHue + ((255 / 4) * 1));
    HelperFillEdges(gHue + ((255 / 4) * 2));
    HelperFillStars(gHue + ((255 / 4) * 3));
    HelperFillPetals(gHue + ((255 / 4) * 4));
}

void SetPie(const unsigned char section, const unsigned char hueInnerPetals, const unsigned char hueOuterPetals, const unsigned char hueStars, const unsigned char hueEdge, const unsigned char hueCorner)
{

    HelperSetPatternSingle(hueInnerPetals, section % PIE_COUNT, PATTERN_PETALS_INNER, COUNT_PETALS_INNER);
    HelperSetPatternSingle(hueOuterPetals, section % PIE_COUNT, PATTERN_PETALS_OUTER, COUNT_PETALS_OUTER);
    HelperSetPatternSingle(hueStars, section % PIE_COUNT, PATTERN_STARS_OUTER, COUNT_STARS_OUTER);
    HelperSetPatternSingle(hueCorner, section % PIE_COUNT, PATTERN_CORNERS, COUNT_CORNERS);

    // There are more edges then the other elements.
    HelperSetPatternSingle(hueEdge, (section % PIE_COUNT) * 2 + 0, PATTERN_EDGES, COUNT_EDGES);
    HelperSetPatternSingle(hueEdge, (section % PIE_COUNT) * 2 + 1, PATTERN_EDGES, COUNT_EDGES);
}

void RotatingPie()
{
    static const unsigned char SPEED = 250;
    fadeToBlackBy(leds, NUM_LEDS, 10);

    static unsigned char offset = 0;

    HelperSetSingle(gHue, LED_CENTER_START);
    SetPie(offset, gHue + 8, gHue + 16, gHue + 24, gHue + 32, gHue + 40);
    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED;
        offset++;
        if (offset > PIE_COUNT) {
            offset = 1;
        }
    }
}

void RandomPie()
{
    fadeToBlackBy(leds, NUM_LEDS, 1);

    static const unsigned char SPEED = 200;

    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED;
        SetPie(random8(PIE_COUNT), gHue + 8, gHue + 16, gHue + 24, gHue + 32, gHue + 40);
    }
}





void RoatingBars() 
{
    const unsigned short SPEED = 300;
    const unsigned char BARS_COUNT = 6;
    const unsigned char bar[24] = {26, 17, 12, 6, 13, 17, 25, 30, 26, 25, 24, 23, 30, 24, 19, 10, 23, 19, 11, 6, 10, 11, 12, 13 };

    fadeToBlackBy(leds, NUM_LEDS, 1);

    static unsigned long offset = 0;
    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED;
        offset ++;
        if (offset >= BARS_COUNT) {
            offset = 0;
        } 
    }

    const unsigned short SIZE_OF_BAR = 4 ; 
    for( unsigned char pixelOffset = 0 ; pixelOffset < SIZE_OF_BAR ; pixelOffset++ ) {
         HelperSetSingle(gHue, bar[ offset * 4 + pixelOffset ]) ; 
    }
}


void CycleInnward() {
    const unsigned short SPEED = 50;
    const unsigned char SNAKE_COUNT = 38;

    const unsigned char snake[] = {0,1,2,3,4,14,15,27,28,36,35,34,33,32,22,21,9,8,7,6,5,13,16,26,29,30,31,23,20,10,11,12,17,25,24,19,18}; 

    static unsigned long offset = 0;
    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED;
        offset ++;
        if (offset >= SNAKE_COUNT) {
            offset = 0;
        } 
        
    }
    HelperSetSingle(gHue, snake[ offset ]) ; 
    fadeToBlackBy(leds, NUM_LEDS, 2);    
}


void Circles() {
    
    const unsigned short SPEED = 50;
    char direction = 1;

    const unsigned char OUTSIDE_COUNT = 18;
    const unsigned char outside[] = {0,1,2,3,4,14,15,27,28,36,35,34,33,32,22,21,9,8}; 
    const unsigned char MIDDLE_COUNT = 12;
    const unsigned char middle[] = {7,6,5,13,16,26,29,30,31,23,20,10};    
    const unsigned char INSIDE_COUNT = 6;
    const unsigned char inside[] = {11,12,17,25,24,19}; 

    static unsigned long offset = 0;
    static unsigned long nextUpdated = 0;
    if (nextUpdated < millis()) {
        nextUpdated = millis() + SPEED;
        offset = offset + direction ;
        if (offset >= 18 && direction > 0 || offset <= 0 && direction < 0 ) {
            direction = direction * -1 ; 
        } 
        
    }
    HelperSetSingle(gHue + ((255 / 4) * 1), outside[ offset % OUTSIDE_COUNT ]) ; 
    HelperSetSingle(gHue + ((255 / 4) * 2), middle[ MIDDLE_COUNT - (offset % MIDDLE_COUNT) -1 ]) ; 
    HelperSetSingle(gHue + ((255 / 4) * 3), inside[ offset % INSIDE_COUNT ]) ; 
    HelperSetSingle(gHue + ((255 / 4) * 4), LED_CENTER_START);

    fadeToBlackBy(leds, NUM_LEDS, 10);    
}

*/
