#include <Adafruit_NeoPixel.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#define PIN 26

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

Adafruit_LIS3DH lis = Adafruit_LIS3DH();

// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define CLICKTHRESHHOLD 40

typedef enum State {
    STATE_IDLE,
    STATE_WORKING,
    STATE_PRE_BREAK,
    STATE_BREAK
} STATE;

typedef enum Side {
    SIDE_UP,
    SIDE_DOWN
} Side;

State current_state;
uint8_t timer = 0;

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'
  if (!lis.begin(0x18)) {
      Serial.println("Failed to initialize acceleration sensor!");
      while (1);
  }
  lis.setRange(LIS3DH_RANGE_4_G);
  lis.setClick(2, CLICKTHRESHHOLD);
  current_state = STATE_IDLE;
}

void loop() {
  sensors_event_t event;
  static unsigned long previous_millis = 0;
  unsigned long current_millis;
  Side current_side;
  static Side previous_side = SIDE_UP;
  uint8_t click;
  bool flipped = false;
  bool timeout = false;

  lis.getEvent(&event);
  click = lis.getClick();
  Serial.print("\t\tX:  "); Serial.print(event.acceleration.x);
  Serial.print("  \tY:  "); Serial.print(event.acceleration.y);
  Serial.print("  \tZ:  "); Serial.print(event.acceleration.z);
  Serial.println(" m/s^2");
  Serial.print("click: ");
  Serial.print(click, HEX);
  Serial.println();

  if (event.acceleration.z > 5.0) {
      current_side = SIDE_UP;
  } else if (event.acceleration.z < -5.0) {
      current_side = SIDE_DOWN;
  }

  if (current_side != previous_side) {
      flipped = true;
      previous_side = current_side;
  }

  current_millis = millis();
  if ((timer > 0) && (current_millis - previous_millis) > (timer * 1000))
  {
      timeout = true;
  }

  Serial.print("State: ");
  Serial.println(current_state);

  switch (current_state) {
      case STATE_IDLE:
          if (click & 0x30) {
              current_state = STATE_WORKING;
              timer = 25;
              previous_millis = millis();
          } else {
              colorLed(strip.Color(0, 0, 0));
          }
          break;

      case STATE_WORKING:
          if (timeout == true) {
              current_state = STATE_PRE_BREAK;
              timeout = false;
              timer = 5;
              previous_millis = millis();
          } else if (flipped == true) {
              current_state = STATE_BREAK;
              timeout = false;
              timer = 1;
              previous_millis = millis();
          } else {
              colorLed(strip.Color(255, 0, 0));
          }
          break;

      case STATE_PRE_BREAK:
          if (timeout == true) {
              current_state = STATE_BREAK;
              timeout = false;
              timer = 1;
              previous_millis = millis();
          } else {
              colorLed(strip.Color(0, 255, 0));
          }
          break;

      case STATE_BREAK:
          if (timeout == true) {
              current_state = STATE_IDLE;
              timeout = false;
              timer = 0;
          } else {
              colorLed(strip.Color(0, 0, 255));
          }
          break;

      default:
          break;
  }
  delay(1000);
}

void colorLed(uint32_t c) {
    strip.setPixelColor(0, c);
    strip.show();
}
