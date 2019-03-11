#### Introduction
    This project implements a pomodoro timer using LED as indicator of working status.
    The timer is controled by fliping or tapping on it.

#### Hardware:
  * Adafruit ESP32 board:
    https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/overview
  * Flora NeoPixel V2
  * Adafruit LIS3DH 3-Axis accelerameter
    https://www.adafruit.com/product/2809

#### Connection:
  * Adafruit ESP32 board:
    3V         ---> Flora NeoPixel + && Adafruit LIS3DH Vin
    GND        ---> Flora NeoPixel - && Adafruit LIS3DH GND
    A0/GPIO26  ---> Flora NeoPixel data in
    SCL        ---> Adafruit LIS3DH SCL
    SDA        ---> Adafruit LIS3DH SDA

#### Resources:
  * Adafruit NeoPixel driver:
     https://github.com/adafruit/Adafruit_NeoPixel
     To control Adafruit FLORA RGB smart pixel
  * Arduino core for ESP32:
     https://github.com/espressif/arduino-esp32
  * ESP32 technical manual:
     https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
  * LIS3DH Manual
    https://www.st.com/resource/en/datasheet/cd00274221.pdf
    Adafruit LIS3DH breakout board:
    https://cdn-learn.adafruit.com/downloads/pdf/adafruit-lis3dh-triple-axis-accelerometer-breakout.pdf
  * Arduino time -- millies:
    https://www.arduino.cc/reference/en/language/functions/time/millis/

