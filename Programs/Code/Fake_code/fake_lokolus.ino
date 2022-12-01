#define NUM_LEDS_TOP 11
#define NUM_LEDS_BOT 9

#include "FastLED.h"
#define LED_PIN_T 6
#define LED_PIN_B 7

CRGB ledsT[NUM_LEDS_TOP];
CRGB ledsB[NUM_LEDS_BOT];
int counter = 0;
bool direction;

void setup() {
  FastLED.addLeds<WS2812, LED_PIN_T, GRB>(ledsT, NUM_LEDS_TOP);
  FastLED.addLeds<WS2812, LED_PIN_B, GRB>(ledsB, NUM_LEDS_BOT);
  FastLED.setBrightness(255);
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}
void loop() {
  for (int i = 0; i < NUM_LEDS_TOP; i++ ) {         // от 0 до первой трети
    ledsT[i] = CHSV(235, 255, counter);  // HSV. Увеличивать HUE (цвет)
    ledsB[i] = CHSV(235, 255, counter); 
    // умножение i уменьшает шаг радуги
  }
  
  if (direction){
    counter++;
  }else{
    counter--;
  }
  if (counter >= 255){
    direction = false;
    counter = 255;
  } else  if (counter <= 25) {
    direction = true;
    counter = 25;
  }
  Serial.println(direction);
  FastLED.show();
  delay(50);         // скорость движения радуги
}