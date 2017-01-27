/*
* Copyright (C) 2017 Nick Naumenko (https://github.com/nnaumenko)
* All rights reserved
* This software may be modified and distributed under the terms
* of the MIT license. See the LICENSE file for details.
*/



#include "hardware.h"
#include "neopixel.h"

/// @brief Set up a neopixel array for use
void Neopixel::begin(void) {
  ///Set neopixel pin to output mode
  bitSet(HW_NEOPIXEL_DIR, HW_NEOPIXEL_BIT);
}

/// @brief Set all available neopixels to the same colour
/// @param r Red component, range 0..255
/// @param g Green component, range 0..255
/// @param b Blue component, range 0..255
void Neopixel::setUniformColour(uint8_t r, uint8_t g, uint8_t b) {
  noInterrupts();
  for (int i = 0; i < HW_NEOPIXEL_NUMBER; i++) {
    sendRgb(r, g, b);
  }
  show();
  noInterrupts();

}

