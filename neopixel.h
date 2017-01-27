/*
* Copyright (C) 2017 Nick Naumenko (https://github.com/nnaumenko)
* All rights reserved
* This software may be modified and distributed under the terms
* of the MIT license. See the LICENSE file for details.
*/

/// @file
/// @brief Fast neopixel array control
///
/// Based on https://github.com/bigjosh/SimpleNeoPixelDemo
///
/// The fast control principle is described here: https://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/

#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include <Arduino.h>

/// @defgroup neopixel Array of neopixels (WS2812).
/// @brief Allows controlling array of neopixels.
///
/// Provides Neopixel class to individually set RGB values of each neopixel
///
/// This module also contains all macros used by Neopixel class as a compile-time settings
///
/// @{

/// @brief Used for the fast control of neopixel array
/// 
/// 
class Neopixel {
  public:
    void begin(void);
    void setUniformColour(uint8_t r, uint8_t g, uint8_t b);
    void setFromArray(uint8_t r[], uint8_t g[], uint8_t b[]);
  private:
    inline void sendBit(bool bitVal);
    inline void sendByte(uint8_t input);
    inline void sendRgb(uint8_t r, uint8_t g, uint8_t b);
    inline void show(void);
};

/// @}

//////////////////////////////////////////////////////////////////////
// Neopixel inline methods
//////////////////////////////////////////////////////////////////////

/// @brief Sends a single bit to neopixel array
/// @param bitValue Bit to send
/// @warning The interrupts must be turned off when the 0 bit is being sent 
/// (HW_NEOPIXEL_T0H + HW_NEOPIXEL_T0L, ~1 us)
/// @warning Total time of interrupts and time of pixel generation must not exceed reset 
/// time (HW_NEOPIXEL_RES, ~5 us)
void Neopixel::sendBit(bool bitValue) {
  if (bitValue) {//bit value 0
    asm volatile (
      "sbi %[port], %[bit] \n\t"        // Set the output bit
      ".rept %[onCycles] \n\t"          // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      "cbi %[port], %[bit] \n\t"        // Clear the output bit
      ".rept %[offCycles] \n\t"         // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      ::
      [port] "I" (_SFR_IO_ADDR(HW_NEOPIXEL_PORT)),
      [bit] "I" (HW_NEOPIXEL_BIT),
      //1-bit width less overhead  for the actual bit setting
      //Note that this delay could be longer and everything would still work
      [onCycles] "I" (NS_TO_CYCLES(HW_NEOPIXEL_T1H) - 2),
      //Minimum interbit delay. Note that we probably don't need this at all
      //since the loop overhead will be enough
      [offCycles] "I" (NS_TO_CYCLES(HW_NEOPIXEL_T1L) - 2)
    );
  }
  else {//bit value 1
    //time-critical code
    //disable interrupts here if they are not disabled outside this method
    asm volatile (
      "sbi %[port], %[bit] \n\t"                              // Set the output bit
      ".rept %[onCycles] \n\t"                                // Now timing actually matters. The 0-bit must be long enough to be detected but not too long or it will be a 1-bit
      "nop \n\t"                                              // Execute NOPs to delay exactly the specified number of cycles
      ".endr \n\t"
      "cbi %[port], %[bit] \n\t"                              // Clear the output bit
      ".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      ::
      [port] "I" (_SFR_IO_ADDR(HW_NEOPIXEL_PORT)),
      [bit] "I" (HW_NEOPIXEL_BIT),
      [onCycles] "I" (NS_TO_CYCLES(HW_NEOPIXEL_T0H) - 2),
      [offCycles] "I" (NS_TO_CYCLES(HW_NEOPIXEL_T0L) - 2)
    );
    //enable interrupts if disabled previously
  }
}

/// @brief Sends a byte to neopixel array
/// @param input Byte to send
void Neopixel::sendByte(uint8_t input) {
  // Note that the inter-bit gap can be as long as you want as long as it doesn't exceed the 5us reset timeout (which is a long time)
  // Here I have been generous and not tried to squeeze the gap tight but instead erred on the side of lots of extra time.
  // Neopixel wants bit in highest-to-lowest order
  for (uint8_t bit = 0; bit < 8; bit++) {
    sendBit(bitRead(input, 7));
    input <<= 1;
  }
}

/// @brief Sends a single RGB triplet to neopixel array
/// @param r Red component, range 0..255
/// @param g Green component, range 0..255
/// @param b Blue component, range 0..255
void Neopixel::sendRgb(uint8_t r, uint8_t g, uint8_t b) {
  //Neopixel rgb components order is green then red then blue
  sendByte(g);
  sendByte(r);
  sendByte(b);
}

/// @brief Makes neopixels actually display the RGB values previously sent to them
void Neopixel::show(void) {
  // Delay must be AT LEAST this long (too short might not work, too long not a problem)
  const uint32_t nanosecPerMiscrosec = 1000UL;
  delayMicroseconds((HW_NEOPIXEL_RES / nanosecPerMiscrosec) + 1);
}

#endif // #ifndef NEOPIXEL_H
