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

// The status LED not part of the patterns.
const unsigned char LED_OFFSET = 1;
const unsigned char LED_CENTER_START = 18;
const unsigned char PIE_COUNT = 6;

const unsigned char PATTERN_CORNERS[] = { 0, 3, 15, 36, 33, 21 };
const unsigned char PATTERN_STARS[] = { 5, 7, 16, 18, 20, 29, 31 };
const unsigned char PATTERN_PETALS[] = { 6, 10, 11, 12, 13, 17, 19, 23, 24, 25, 26, 30 };

const unsigned char PATTERN_EDGES[] = { 1, 2, 4, 14, 27, 28, 35, 34, 32, 22, 9, 8 };

const unsigned char PATTERN_STARS_OUTER[] = { 7, 5, 16, 29, 31, 20 };
const unsigned char PATTERN_PETALS_OUTER[] = { 6, 13, 26, 30, 23, 10 };
const unsigned char PATTERN_PETALS_INNER[] = { 11, 12, 17, 25, 24, 19 };

const unsigned char COUNT_CORNERS = 6;
const unsigned char COUNT_EDGES = 12;
const unsigned char COUNT_STARS = 7;
const unsigned char COUNT_PETALS = 12;

const unsigned char COUNT_STARS_OUTER = 6;
const unsigned char COUNT_PETALS_OUTER = 6;
const unsigned char COUNT_PETALS_INNER = 6;

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
SimplePatternList gPatterns = { Circles ,CycleInnward, Snake, RotatingPie, TypeHueRotate, RoatingBars, RotatingPie, RandomGlow, RandomPie, rainbow, sinelon, juggle, bpm }; 

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

void HelperFillCorners(unsigned char hue)
{
    for (unsigned short offset = 0; offset < COUNT_CORNERS; offset++) {
        leds[LED_OFFSET + PATTERN_CORNERS[offset]] = CHSV(hue, 200, 255);
    }
}
void HelperFillEdges(unsigned char hue)
{
    for (unsigned short offset = 0; offset < COUNT_EDGES; offset++) {
        leds[LED_OFFSET + PATTERN_EDGES[offset]] = CHSV(hue, 200, 255);
    }
}
void HelperFillStars(unsigned char hue)
{
    for (unsigned short offset = 0; offset < COUNT_STARS; offset++) {
        leds[LED_OFFSET + PATTERN_STARS[offset]] = CHSV(hue, 200, 255);
    }
}
void HelperFillPetals(unsigned char hue)
{
    for (unsigned short offset = 0; offset < COUNT_PETALS; offset++) {
        leds[LED_OFFSET + PATTERN_PETALS[offset]] = CHSV(hue, 200, 255);
    }
}

void HelperSetSingle(const unsigned char hue, const unsigned char offset)
{
    if (offset > NUM_LEDS) {
        return; // Out of range.
    }
    leds[LED_OFFSET + offset] = CHSV(hue, 200, 255);
}
void HelperSetPatternSingle(const unsigned char hue, const unsigned char offset, const unsigned char* ledOffsets, const unsigned char maxCount)
{
    HelperSetSingle(hue, ledOffsets[offset % maxCount]);
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

void Snake() {
    const unsigned short SPEED = 50;
    const unsigned char SNAKE_COUNT = 37;
    
    const unsigned char snake[] = {6,2,3,5,12,13,4,14,15,16,17,26,27,28,36,29,25,30,35,34,33,31,24,23,32,22,21,20,19,10,9,8,0,1,7,11};
    
    HelperSetSingle(gHue, LED_CENTER_START);

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
    fadeToBlackBy(leds, NUM_LEDS, 1);
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

