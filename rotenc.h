/*
* Copyright (C) 2017 Nick Naumenko (https://github.com/nnaumenko)
* All rights reserved
* This software may be modified and distributed under the terms
* of the MIT license. See the LICENSE file for details.
*/

/// @file
/// @brief Components to use a Rotary Encoder as UI control

#ifndef ROTENC_H
#define ROTENC_H

#include <Arduino.h>

#include "hardware.h"

/// @defgroup rot_enc_control Rotary Encoder Control
/// @brief Allows using rotary encoder as a user interface controller
///
/// Provides RotEnc class to use rotary encoder as a user interface controller (by turning
/// encoder's shaft and short clicking / long clicking encoder button
///
/// This module also contains all macros used by RotEnc class as a compile-time settings
///
/// @{

/// @brief Provides a way to control various user interfaces with a rotary encoder
///
/// 16-bit signed counter is incremented/decremented within specified range by rotating
/// encoder's shaft
///
/// When encoder button is clicked, short and long clicks are detected
///
/// Implementation based on lookup table approach described here:
/// https://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino/
///
/// Interrupt handlers must be called externally from the corresponding ISR, e.g.:
/// @code
/// ISR (HW_ROTENC_INTVECT) {
///  rotenc.encoderInterruptHandler();
///  rotenc.buttonInterruptHandler();
///}
/// @endcode
///
/// @warning The following limitations apply:
/// * All controller pins connected to rotary encoser must share the same port and the same
/// Pin Change interrupt vector
/// * This implementation uses macros to setup ports, pins, registers, etc (see hardware.h)
/// in order to economise on RAM thus there is only one rotary encoder allowed per device

class RotEnc {
  public:
    RotEnc();
    void begin(void);
  public:
    inline int16_t getCounter(void);
    bool setCounter (int16_t counter, int16_t minLimit, int16_t maxLimit, bool wrap);
  public:
    /// State of the rotary encoder's button
    enum ButtonState {
      BUTTON_NONE,            ///< Button not pressed.
      BUTTON_SHORT_CLICK,     ///< Short click on the button.
      BUTTON_LONG_CLICK,      ///< Long click on the button.
    };
    inline ButtonState getButtonState(void);
  public:
    inline void encoderInterruptHandler(void);
    inline void buttonInterruptHandler(void);
  private:
    static const int16_t INT16_T_MIN = -32768;
    static const int16_t INT16_T_MAX = 32767;
    static const int16_t minLimitRange = (INT16_T_MIN / HW_ROTENC_CYCLES_PER_DETENT /*+ 2*/ + 1);
    static const int16_t maxLimitRange = INT16_T_MAX / HW_ROTENC_CYCLES_PER_DETENT /*- 2*/ - 1;
  private:
    volatile int16_t counter;
    int16_t counterMinLimit;
    int16_t counterMaxLimit;
    bool counterWrap;
  private:
    ButtonState buttonState;
};

/// @}

//////////////////////////////////////////////////////////////////////
// RotEnc inline methods
//////////////////////////////////////////////////////////////////////

/// @brief Get rotary encoder's counter value
/// @return Rotary encoder's counter value within range set by setcounter()
int16_t RotEnc::getCounter(void) {
  noInterrupts();
  int16_t retVal = counter / HW_ROTENC_CYCLES_PER_DETENT;
  interrupts();
  return (retVal);
}

/// @brief Call this method from the corresponding ISR
///
/// Updates counter when encoder shaft is rotated
void RotEnc::encoderInterruptHandler(void) {
  static const PROGMEM int8_t statesTable[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
  static const uint8_t fourLowestBits = 0x0f;
  static uint8_t oldState = 0;
  oldState <<= 2;
  oldState |= (bitRead(HW_ROTENC_PORT, HW_ROTENC_B_BIT) << 1) | bitRead(HW_ROTENC_PORT, HW_ROTENC_A_BIT);
  oldState &= fourLowestBits;
  int8_t increment = pgm_read_byte(&statesTable[oldState]);
  if ((counter == counterMaxLimit) && (increment > 0)) {
    if (counterWrap)
      counter = counterMinLimit;
    else
      counter = counterMaxLimit;
    increment = 0;
  }
  if ((counter == counterMinLimit) && (increment < 0)) {
    if (counterWrap)
      counter = counterMaxLimit;
    else
      counter = counterMinLimit;
    increment = 0;
  }
  counter += increment;
}

/// @brief Call this method from the corresponding ISR
///
/// Detect encoder button long and short clicks
void RotEnc::buttonInterruptHandler(void) {
  uint8_t currentButtonState = bitRead(HW_ROTENC_PORT, HW_ROTENC_BTN_BIT);
  static bool oldButtonState = true;
  static uint32_t buttonPressTime = 0;
  if (!currentButtonState && oldButtonState) { // button pressed
    buttonPressTime = millis();
  }
  if (currentButtonState && !oldButtonState) { // button released
    uint32_t buttonHoldTime = millis() - buttonPressTime;
    if (buttonHoldTime > HW_BUTTON_SHORT_CLICK_MIN_TIME) buttonState = BUTTON_SHORT_CLICK;
    if (buttonHoldTime > HW_BUTTON_LONG_CLICK_MIN_TIME) buttonState = BUTTON_LONG_CLICK;
  }
  oldButtonState = currentButtonState;
}

/// @brief Detects long & short clicks of the rotary encoder button
///
/// If encoder's button short or long click was detected since previous call, this
/// method returns corresponding RotEnc::ButtonState
///
/// @return RotEnc::BUTTON_SHORT_CLICK, RotEnc::BUTTON_LONG_CLICK or BUTTON_NONE if
/// no button click occured since previous call
///
RotEnc::ButtonState RotEnc::getButtonState(void) {
  noInterrupts();
  ButtonState tempState = buttonState;
  buttonState = BUTTON_NONE;
  interrupts();
  return (tempState);
}
#endif // #ifndef ROTENC_H
