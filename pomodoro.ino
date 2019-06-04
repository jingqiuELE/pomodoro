#include <Adafruit_NeoPixel.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include "pomodoro.h"

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
#define DEBUG 0

STATE current_state;
unsigned long previous_millis = 0;

void setup() {
  Serial.begin(115200);
  print_wakeup_reason();
  if (!lis.begin(0x18)) {
      Serial.println("Failed to initialize acceleration sensor!");
      while (1);
  }
  lis.setRange(LIS3DH_RANGE_4_G);
  lis.setClick(2, CLICKTHRESHHOLD);
  switchState(STATE_WORKING, &current_state);
  strip.begin();
  strip.setBrightness(50);
  strip.show();
}

void loop() {
  sensors_event_t event;
  unsigned long current_millis;
  static SIDE previous_side = SIDE_UP;
  SIDE current_side;
  uint8_t click;
  bool flipped = false;
  bool timeout = false;
  bool clicked = false;

  lis.getEvent(&event);
  click = lis.getClick();
#if DEBUG
  Serial.print("\t\tX:  "); Serial.print(event.acceleration.x);
  Serial.print("  \tY:  "); Serial.print(event.acceleration.y);
  Serial.print("  \tZ:  "); Serial.print(event.acceleration.z);
  Serial.println(" m/s^2");
  Serial.print("click: ");
  Serial.println(click, HEX);
  Serial.println();
#endif

  if (click & 0x40) {
      clicked = true;
  } else {
      clicked = false;
  }

  if (event.acceleration.z > 5.0) {
      current_side = SIDE_UP;
  } else if (event.acceleration.z < -5.0) {
      current_side = SIDE_DOWN;
  }

  if (current_side != previous_side) {
      flipped = true;
      previous_side = current_side;
  } else {
      flipped = false;
  }

  current_millis = millis();
  if ((current_state.timeout > 0) &&
      (current_millis - previous_millis) > current_state.timeout) {
      timeout = true;
  } else {
      timeout = false;
  }

  switch (current_state.state_id) {
      case STATE_IDLE:
          if (clicked == true) {
              if (current_side == SIDE_UP) {
                  switchState(STATE_WORKING, &current_state);
              } else {
                  switchState(STATE_BREAK, &current_state);
              }
          }
          if (timeout == true) {
              switchState(STATE_SLEEP, &current_state);
          }
          break;

      case STATE_WORKING:
          if (timeout == true) {
              switchState(STATE_PRE_BREAK, &current_state);
          } else if (flipped == true) {
              switchState(STATE_BREAK, &current_state);
          }
          break;

      case STATE_PRE_BREAK:
          if (timeout == true) {
              switchState(STATE_BREAK, &current_state);
          }
          break;

      case STATE_BREAK:
          if (timeout == true || flipped == true) {
              switchState(STATE_IDLE, &current_state);
          }
          break;

      default:
          break;
  }

  if (current_state.ledBlink == true) {
      blinkLed(current_state.ledColor, 500);
  }

  delay(50);
}

void switchState(STATE_ID new_state_id, STATE *current_state) {
//#if DEBUG
    Serial.print("State: ");
    Serial.println(new_state_id);
//#endif
    current_state->state_id = new_state_id;
    switch (new_state_id) {
        case STATE_IDLE:
            current_state->timeout = 30 * 1000;
            current_state->ledColor = strip.Color(0, 0, 0);
            current_state->ledBlink = false;

            break;

        case STATE_WORKING:
            current_state->timeout = 25 * 60 * 1000;
            current_state->ledColor = strip.Color(255, 0, 0);
            current_state->ledBlink = false;
            break;

        case STATE_PRE_BREAK:
            current_state->timeout = 5 * 60 * 1000;
            current_state->ledColor = strip.Color(255, 218, 0);
            current_state->ledBlink = false;
            break;

        case STATE_BREAK:
            current_state->timeout = 30 * 1000;
            current_state->ledColor = strip.Color(0, 255, 0);
            current_state->ledBlink = true;
            break;

        case STATE_SLEEP:
            sleep();
            break;

        default:
            break;
    }

    previous_millis = millis();
    if (current_state->ledBlink == false) {
        colorLed(current_state->ledColor);
    }
}

void colorLed(uint32_t c) {
    Serial.print("colorLed: ");
    Serial.println(c, HEX);
    strip.setPixelColor(0, c);
    strip.show();
}

void blinkLed(uint32_t c, unsigned long interval) {
    static unsigned long previous = 0;
    uint32_t color;
    unsigned long current;

    current = millis();
    if ((current - previous) >= interval) {
        previous = current;
        color = strip.getPixelColor(0);
        if (color != 0) {
            colorLed(strip.Color(0, 0, 0));
        } else {
            colorLed(c);
        }
    }
}

void sleep(void) {
  sensors_event_t event;
  //lis3dh_configure_int();
  lis3dh_sleep();
  /*
  We configure the wake up source
  We set our ESP32 to wake up for an external trigger.
  There are two types for ESP32, ext0 and ext1 .
  ext0 uses RTC_IO to wakeup thus requires RTC peripherals
  to be on while ext1 uses RTC Controller so doesnt need
  peripherals to be powered on.
  Note that using internal pullups/pulldowns also requires
  RTC peripherals to be turned on.
  */
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1); //1 = High, 0 = Low
  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
}

/*
 Configure LIS3DH into low power mode
 */
void lis3dh_sleep(void) {
   /* Set odr=400HZ, low-power mode, enable x,y,z axises. */
   lis.writeRegister8(LIS3DH_REG_CTRL1, 0x7F);
   /* Disable High res as this doesn't work with Low power mode.*/
   lis.writeRegister8(LIS3DH_REG_CTRL4, 0x80);
}

void lis3dh_configure_int(void) {
   /*
   Configure wake-up events for INT1 on lis3dh.
   INT1 is then connected to GPIO 33 of esp32 board.
   The interrupt is high effective.
   */
   lis.writeRegister8(LIS3DH_REG_INT1THS, 0x0A);
   lis.writeRegister8(LIS3DH_REG_INT1DUR, 0x00);
   lis.writeRegister8(LIS3DH_REG_INT1CFG, 0x2A);
   /* Enable INT1 for l1_IA1 */
   lis.writeRegister8(LIS3DH_REG_CTRL3, 0x40);
   /* Latch interrupt request on INT1_SRC register */
   lis.writeRegister8(LIS3DH_REG_CTRL5, 0x08);
}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
