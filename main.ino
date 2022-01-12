#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN 12
#define NUM_LEDS 120
#define LEDS_PER_STRIP 30

double volLeftAvg = 0;
double volRightAvg = 0;
double smoothVolLeft = 0;
double smoothVolRight = 0;
unsigned long lastRead = 0;
unsigned int count = 0;

unsigned int sampleTime = 30;

CRGB leds[NUM_LEDS];

double approxRollingAverage (double avg, double newSample, int samples) {
    avg -= avg / samples;
    avg += newSample / samples;
    return avg;
}

double map(double x, double inMin, double inMax, double outMin, double outMax) {
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

void setup() {
    FastLED.addLeds<WS2812B, LED_PIN>(leds, NUM_LEDS);

    Serial.begin(9600);

}

void allWhite() {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(255, 255, 255);
    }

}

void fadeLed(int index, double fade) {
    leds[index] = CRGB(
            leds[index].r / fade,
            leds[index].g / fade,
            leds[index].b / fade
        );
}

void clearLeds() {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(0, 0, 0);
    }
}

void setStripPulse(unsigned int stripIndex, unsigned int amount) {
    int minLed = stripIndex * LEDS_PER_STRIP;

    for (int i = 0; i < min(LEDS_PER_STRIP / 2, amount); i++) {
        leds[minLed + LEDS_PER_STRIP / 2 - 1 - i] = CHSV(i * 360/45 + millis() / 50, 255, 255);
        leds[minLed + LEDS_PER_STRIP / 2 + i] = CHSV(i * 360/45 + millis() / 50, 255, 255);
    }
}

void loop() {
    unsigned long currentTime = millis();

    if (currentTime - lastRead > sampleTime) {
        lastRead = currentTime;

        clearLeds();
//        double volLeft = log10(analogRead(A0));
//        double volRight = log10(analogRead(A1));
//
//        volLeftAvg = approxRollingAverage(volLeftAvg, volLeft, 500);
//        smoothVolLeft = approxRollingAverage(volLeftAvg, volLeft, 500);
//
//        volRightAvg = approxRollingAverage(volRightAvg, volRight, 500);
//        smoothVolRight = approxRollingAverage(volRightAvg, volRight, 500);
//
//        map(smoothVolRight, 0, volRightAvg * 2, 0, LEDS_PER_STRIP / 2);

        count += 1;
        int value = floor(7 * sin(currentTime / 200.0) + 8);
 

        setStripPulse(0, value);
        setStripPulse(1, value);
        setStripPulse(2, value);
        setStripPulse(3, value);

        
        FastLED.show();
    }

}
