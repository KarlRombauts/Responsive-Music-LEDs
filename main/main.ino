#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN 12
#define NUM_LEDS 120
#define LEDS_PER_STRIP 30
#define FADE_AMOUNT 10

#define BASELINE 1500.0
#define ROLLING_WINDOW 500

double volAvg = 0;
double volRightAvg = 0;
double smoothVol = 0;
double smoothVolRight = 0;
float maxVol = 0;
float minVol = 4096;

float bias = 1447.0;
float maxPossibleVol = log(bias / 100.0);


float minAvgVol = log(bias) / 10.0;

unsigned long lastRead = 0;
unsigned int count = 0;

unsigned int sampleTime = 1;

CRGB leds[NUM_LEDS];

double approxRollingAverage (double avg, double newSample, int samples) {
    avg -= avg / samples;
    avg += newSample / samples;
    return avg;
}

float approxRollingMax (double currentMax, double avg, double newSample, int samples) {
    float decendedMax = lerp(currentMax, avg, 1.0/samples);
    return max(max(decendedMax, newSample), bias / 20);
}

float approxRollingMin (double currentMin, double avg, double newSample, int samples) {
    float ascendedMin = lerp(currentMin, avg, 1.0/samples);
    return min(ascendedMin, newSample);
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

int getValue(int i) {
  return max(leds[i].r, max(leds[i].b, leds[i].g));
}

void clearLeds() {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(0, 0, 0);
    }
}

double lerp(double start, double end, double amt){
  return (1 - amt) * start + amt * end;
}

void setStripPulse(unsigned int stripIndex, unsigned int amount, int phaseShift) {
    int minLed = stripIndex * LEDS_PER_STRIP;

    for (int i = 0; i < LEDS_PER_STRIP / 2; i++) {
        if (i <= amount) {
          CHSV color = CHSV(i * 360/45 + millis() / 50 + phaseShift, 255, 255);
          leds[minLed + LEDS_PER_STRIP / 2 - 1 - i] = color;
          leds[minLed + LEDS_PER_STRIP / 2 + i] = color;
        }

        if (i > amount) {
          leds[minLed + LEDS_PER_STRIP / 2 - 1 - i].fadeToBlackBy(FADE_AMOUNT);
          leds[minLed + LEDS_PER_STRIP / 2 + i].fadeToBlackBy(FADE_AMOUNT);
        }
    }
}

float inputToVol(int input) {
  return log(input / 100.0 + 1);
}

float clamp(float input, float minValue, float maxValue) {
    return max(minValue, min(maxValue, input));
}

float mapVolToLeds(float vol, float relativeMin, float relativeMax) {

  float clampedVol = clamp(vol, relativeMin, relativeMax);
  
  return map(clampedVol, 0, relativeMax, 0, 15);
}

void loop() {
    unsigned long currentTime = millis();

    if (currentTime - lastRead > sampleTime) {
        lastRead = currentTime;


        int input = abs(analogRead(A0) - bias);
        
        smoothVol = approxRollingAverage(smoothVol, input, 10);
        
        volAvg = approxRollingAverage(volAvg, smoothVol, ROLLING_WINDOW);
        
        float logInput = inputToVol(smoothVol); // range 0 - 3.16
        float logAvg = max(inputToVol(volAvg), minAvgVol);


        minVol = approxRollingMin(minVol, volAvg, smoothVol, ROLLING_WINDOW);
        maxVol = approxRollingMax(maxVol, volAvg, smoothVol, ROLLING_WINDOW);
        
        float vol = mapVolToLeds(inputToVol(smoothVol), 0, maxPossibleVol);
        float relativeVol = mapVolToLeds(inputToVol(smoothVol), inputToVol(minVol), inputToVol(maxVol));
    

//        setStripPulse(0, mapVolToLeds(inputToVol(minVol), 0, maxPossibleVol));
//        setStripPulse(1, mapVolToLeds(inputToVol(maxVol), 0, maxPossibleVol));
//        setStripPulse(2, mapVolToLeds(inputToVol(smoothVol), 0, maxPossibleVol));
        setStripPulse(0, relativeVol, 0);
        setStripPulse(1, relativeVol, 20);
        setStripPulse(2, relativeVol, 40);
        setStripPulse(3, relativeVol, 60);

        
        FastLED.show();
    }

}
