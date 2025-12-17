#include <DOPLED.h>

#define LED_PIN 2
#define NUM_LEDS 125 // Cannot exceed 256 (8 bit address)

DOPLED myString(LED_PIN);

void setup() {
    myString.begin();
    myString.fillAll(0,0,0);
}

void loop() {
    myString.writePixel(0, 255, 0, 0);
    myString.writePixel(NUM_LEDS-1, 0, 0, 0);
    for (uint16_t i=1; i<NUM_LEDS; i++) {
        myString.writePixel(i, 255, 0, 0);
        myString.writePixel(i-1, 0, 0, 0);
    }
}