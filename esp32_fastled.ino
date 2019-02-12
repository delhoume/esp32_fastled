#include <FastLED.h>

const uint8_t width = 8;
const uint8_t height = 8;

const uint16_t numleds = width * height;

// which pin on esp32 ?
#define LED_PIN 16

CRGB leds[numleds];


const uint8_t kSquareWidth  = width;
const uint8_t kBorderWidth = 1;

TaskHandle_t FastLEDTaskHandle;
TaskHandle_t userTaskHandle = 0;

// could use semaphores but direct call is faster
void FastLEDTask(void* param) {
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // do stuff
        FastLED.show();
        userTaskHandle = 0; // make sure only one is called, is it necessary ?
    }
}

void FastLEDShowESP32() {
  if (userTaskHandle == 0) { // make sure only one is called
    userTaskHandle = xTaskGetCurrentTaskHandle();
    xTaskNotifyGive(FastLEDTaskHandle);
  }
}

// normal
#if 0
uint16_t XY(uint8_t x, uint8_t y) {
  return y * kSquareWidth + x;
}
#else
// alternate (zigzag)
uint16_t XY(uint8_t x, uint8_t y) {
  uint16_t i;
  if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kSquareWidth - 1) - x;
      i = (y * kSquareWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kSquareWidth) + x;
    }
    return i;
}
#endif

void setup() {
  Serial.begin(1152000);
  Serial.println(xPortGetCoreID()); // default code runs in core 1
  Serial.println(pcTaskGetTaskName(NULL)); 
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, numleds).setCorrection(TypicalSMD5050);
    FastLED.setBrightness(7);
    FastLED.clear();
    FastLED.show();
    xTaskCreatePinnedToCore(
        FastLEDTask,
        "FastLEDTask",
        2048, // stack
        NULL,
        0, // priority
        & FastLEDTaskHandle,
        0   // core
    );
    // scheduler is alredy started, no need to call vTaskStartScheduler()
 }

void loop() {
  uint8_t blurAmount = dim8_raw( beatsin8(3,64,192) );
  blur2d( leds, kSquareWidth, kSquareWidth, blurAmount);

  // Use three out-of-sync sine waves
  uint8_t  i = beatsin8(  91, kBorderWidth, kSquareWidth-kBorderWidth);
  uint8_t  j = beatsin8( 109, kBorderWidth, kSquareWidth-kBorderWidth);
  uint8_t  k = beatsin8(  73, kBorderWidth, kSquareWidth-kBorderWidth);
  
  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis();  
  leds[XY( i, j)] += CHSV( ms / 29, 200, 255);
  leds[XY( j, k)] += CHSV( ms / 41, 200, 255);
  leds[XY( k, i)] += CHSV( ms / 73, 200, 255);
  
  FastLEDShowESP32(); // do not block
  delay(10);}
