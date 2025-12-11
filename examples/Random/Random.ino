#include <DOPLED.h>

#define LED_PIN 2

DOPLED myString(LED_PIN);

void setup() {
    myString.begin();
    myString.fill(0,0,0);
}

void loop() {
    // 1/2 Red, 1/2 Green random fill
    myString.setRandom(
        0b0001, // 4 bit mask, indicates which bits of index to use
        0b0000, // 4 bit index, identifies group number
        255, 0, 0 // Red, Green, Blue
    );
    myString.setRandom(
        0b0001, // 4 bit mask, indicates which bits of index to use
        0b0001, // 4 bit index, identifies group number
        0, 255, 0 // Red, Green, Blue
    );

    delay(1000);

    // Fade Green to White
    for (uint16_t g = 255; g<=255; g-=5) { //Underflows, loop ends
        myString.setRandom(
            0b0001, // Use the same mask as above
            0b0001, // Use the same group ID as green above
            0, g, 0
        );
    }

    for (uint16_t w = 0; w<=255; w+=5) {
        myString.setRandom(
            0b0001, // Use the same mask as above
            0b0001, // Use the same group ID as green above
            w, w, w
        );
    }

    delay(1000);

    // Flash random Red and White
    for (int i=0; i<10; i++) {
        myString.fill(255, 255, 255); // Fill white, resets groups
        myString.setRandom(0b0001, 0b0001, 255, 0, 0); // 1/2 Red

        delay(200);
    }

    delay(1000);

    // Static multicolor, 8 groups
    // 7 = 0b111
    myString.setRandom(7, 0, 255,  40,  30); // Red
    myString.setRandom(7, 1,   0, 180,  60); // Green
    myString.setRandom(7, 2,   0,  90, 255); // Blue
    myString.setRandom(7, 3, 255, 215,  40); // Yellow
    myString.setRandom(7, 4, 255, 120,  20); // Orange
    myString.setRandom(7, 5, 155,   0, 200); // Purple
    myString.setRandom(7, 6, 255,  40,  30); // Red (will appear 2x as often)
    myString.setRandom(7, 7,   0, 180,  60); // Green (will appear 2x as often)

    delay(2000);
}