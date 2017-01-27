/*
* Copyright (C) 2017 Nick Naumenko (https://github.com/nnaumenko)
* All rights reserved
* This software may be modified and distributed under the terms
* of the MIT license. See the LICENSE file for details.
*/

/// @file
/// @brief Main program.
///
/// @warning Current version was only tested with Arduino Nano V3 (ATMega328)
/// @todo Also add support for ATTiny

#include "version.h"
#include "hardware.h"

#include "rotenc.h"
#include "neopixel.h"

#define DEBUG ///< Declaring this macro enables debug output to the serial port

RotEnc rotenc;
Neopixel neopixel;

ISR (HW_ROTENC_INTVECT) {
  rotenc.encoderInterruptHandler();
  rotenc.buttonInterruptHandler();
}

#define COLOUR_RESOLUTION 32                                   ///< Hue resolution per primary colour
#define COLOUR_MAX_HUE (COLOUR_RESOLUTION * 6 - 1)             ///< Maximum hue value
#define COLOUR_MAX 256                                         ///< Full brightness of any RGB component
#define COLOUR_OFF 0                                           ///< RGB component is off
#define COLOUR_FACTOR (COLOUR_MAX/COLOUR_RESOLUTION)           ///< Scale factor to obtain RGB components suitable for neopixels
#define COLOUR_RAMPDOWN(val,max) ((max - val) * COLOUR_FACTOR) ///< Linear ramp-up of rgb component
#define COLOUR_RAMPUP(val,min) ((val - min) * COLOUR_FACTOR)   ///< Linear ramp-down of rgb component
#define COLOUR_MAX_BRIGHTNESS 64                               ///< Max value for brightness

uint8_t neopx_red = 0;        ///< Neopixels' calculated red component
uint8_t neopx_green = 0;      ///< Neopixels' calculated green component
uint8_t neopx_blue = 0;       ///< Neopixels' calculated blue component

uint8_t neopx_hue = 52;                             ///< Hue value for neopixels
uint8_t neopx_brightness = COLOUR_MAX_BRIGHTNESS;   ///< Brightness value for neopixels

/// @brief Calculates RGB colour component for neopixels
/// @param component Original component value in range COLOUR_OFF..COLOUR_MAX
/// @param brightness Brightness value in range 0..COLOUR_MAX_BRIGHTNESS.
/// @return Calculated component value in range 0..255.
uint8_t calcColourComponent(uint16_t component, uint8_t brightness) {
  component = component * (uint16_t)brightness / COLOUR_MAX_BRIGHTNESS;
  return ((component > 255) ? 255 : component);
}

/// @brief Calculates RGB value from hue and stores in neopx_red, neopx_green and neopx_blue.
/// @param hue Hue value in range 0..COLOUR_MAX_HUE.
/// @param brightness Brightness value in range 0..COLOUR_MAX_BRIGHTNESS.
void calcRGB(uint8_t hue, uint8_t brightness) {
  uint16_t r = 0, g = 0, b = 0;
  if (hue >= COLOUR_MAX_HUE) hue = 0;
  if (hue < COLOUR_RESOLUTION) {
    r = COLOUR_MAX;
    g = COLOUR_OFF;
    b = COLOUR_RAMPDOWN(hue, COLOUR_RESOLUTION);
  }
  if ((hue >= COLOUR_RESOLUTION) && (hue < COLOUR_RESOLUTION * 2)) {
    r = COLOUR_MAX;
    g = COLOUR_RAMPUP(hue, COLOUR_RESOLUTION);
    b = COLOUR_OFF;
  }
  if ((hue >= COLOUR_RESOLUTION * 2) && (hue < COLOUR_RESOLUTION * 3)) {
    r = COLOUR_RAMPDOWN(hue, COLOUR_RESOLUTION * 3);
    g = COLOUR_MAX;
    b = COLOUR_OFF;
  }
  if ((hue >= COLOUR_RESOLUTION * 3) && (hue < COLOUR_RESOLUTION * 4)) {
    r = COLOUR_OFF;
    g = COLOUR_MAX;
    b = COLOUR_RAMPUP(hue, COLOUR_RESOLUTION * 3);
  }
  if ((hue >= COLOUR_RESOLUTION * 4) && (hue < COLOUR_RESOLUTION * 5)) {
    r = COLOUR_OFF;
    g = COLOUR_RAMPDOWN(hue, COLOUR_RESOLUTION * 5);
    b = COLOUR_MAX;
  }
  if (hue >= COLOUR_RESOLUTION * 5) {
    r = COLOUR_RAMPUP(hue, COLOUR_RESOLUTION * 5);
    g = COLOUR_OFF;
    b = COLOUR_MAX;
  }
  neopx_red = calcColourComponent(r, brightness);
  neopx_green = calcColourComponent(g, brightness);
  neopx_blue = calcColourComponent(b, brightness);
}

bool controlParameter = false; ///< Parameter controlled by encoder rotation, false for brightness, true for hue
bool lampOn = true;            ///< True if lamp is on, false if lamp is off.

///@brief Sets rotary encoder counter range and start value when parameter controlled by neopixel changed.
void updateControl(void) {
  if (controlParameter) {
    rotenc.setCounter(neopx_hue, 0, COLOUR_MAX_HUE, true);
  }
  else {
    rotenc.setCounter(neopx_brightness, 0, COLOUR_MAX_BRIGHTNESS, false);
  }
}

///@brief Updates neopixels if hue or brightness changed.
void updateNeopixels(void) {
  calcRGB(neopx_hue, lampOn ? neopx_brightness : 0);
  neopixel.setUniformColour(neopx_red, neopx_green, neopx_blue);
#ifdef DEBUG
  Serial.print(F("Neopixels updated, RGB: "));
  Serial.print(neopx_red);
  Serial.print(F(" / "));
  Serial.print(neopx_green);
  Serial.print(F(" / "));
  Serial.print(neopx_blue);
  Serial.print(F(", brightness: "));
  Serial.println(neopx_brightness);
#endif
}

void setup() {
#ifdef DEBUG
  Serial.begin (115200);
  Serial.print(F("Starting...\nFirmware version "));
  Serial.print(FIRMWARE_VERSION_MAJOR);
  Serial.print('.');
  Serial.println(FIRMWARE_VERSION_MINOR);
  if (lampOn)
    Serial.println(F("Lamp on."));
  else
    Serial.println(F("Lamp off."));
  if (controlParameter)
    Serial.println(F("Rotary Encoder shaft controls hue."));
  else
    Serial.println(F("Rotary Encoder shaft controls brightness."));
   Serial.print(F("Start parameters: hue = "));
   Serial.print(neopx_hue);
   Serial.print(F(", brightness = "));
   Serial.println(neopx_brightness);
#endif
  rotenc.begin();
  updateControl();
  neopixel.begin();
  updateNeopixels();
}

void loop() {
  static int16_t oldCounter = 0;
  int16_t currCounter = rotenc.getCounter();
  //Encoder shaft control
  if (currCounter != oldCounter) {
#ifdef DEBUG
    Serial.print(F("Rotary encoder counter: "));
    Serial.println(currCounter, DEC);
#endif
    if (lampOn) {
      if (controlParameter)
        neopx_hue = currCounter;
      else
        neopx_brightness = currCounter;
      updateNeopixels();
    }
    else {
      updateControl();
    }
    oldCounter = currCounter;
  }
  //Encoder button control
  RotEnc::ButtonState buttonState = rotenc.getButtonState();
  if (buttonState == RotEnc::BUTTON_SHORT_CLICK) {
#ifdef DEBUG
    Serial.println(F("Short click detected."));
#endif
    lampOn = !lampOn;
#ifdef DEBUG
    if (lampOn)
      Serial.println(F("Lamp on."));
    else
      Serial.println(F("Lamp off."));
    updateNeopixels();
  }
#endif
  if (buttonState == RotEnc::BUTTON_LONG_CLICK) {
#ifdef DEBUG
    Serial.println(F("Long click detected."));
#endif
    controlParameter = !controlParameter;
    updateControl();
#ifdef DEBUG
    if (controlParameter)
      Serial.println(F("Rotary Encoder shaft now controls hue."));
    else
      Serial.println(F("Rotary Encoder shaft now controls brightness."));
#endif
    updateNeopixels();
  }
}
