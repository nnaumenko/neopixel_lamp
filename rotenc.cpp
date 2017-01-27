/*
* Copyright (C) 2017 Nick Naumenko (https://github.com/nnaumenko)
* All rights reserved
* This software may be modified and distributed under the terms
* of the MIT license. See the LICENSE file for details.
*/

#include "rotenc.h"
#include "hardware.h"

//////////////////////////////////////////////////////////////////////
// RotEnc
//////////////////////////////////////////////////////////////////////

/// @brief Initialises private fields with default values
RotEnc::RotEnc() {
  counter = 0;
  counterMinLimit = minLimitRange;
  counterMaxLimit = maxLimitRange;
  counterWrap = false;
  buttonState = BUTTON_NONE;
}

/// @brief Sets up rotary encoder class before use
///
/// Sets pins corresponding to encoder lines A & B, and encoder switch
/// to input mode and enables pull-up on these pins
///
/// Sets bits in Pin Change Interrupt registers corresponding to
/// encoder pins in order to activate their Pin Change Interrupts
///
void RotEnc::begin(void) {
  //Set pins corresponding to lines A and B to input mode
  bitClear(HW_ROTENC_DIR, HW_ROTENC_A_BIT);
  bitClear(HW_ROTENC_DIR, HW_ROTENC_B_BIT);
  //Enable pullup on lines A and B
  bitSet(HW_ROTENC_OUT, HW_ROTENC_A_BIT);
  bitSet(HW_ROTENC_OUT, HW_ROTENC_B_BIT);
  //Setup pin change interrupt registers
  noInterrupts();
  bitSet(HW_ROTENC_PCMSK, HW_ROTENC_A_INT);
  bitSet(HW_ROTENC_PCMSK, HW_ROTENC_B_INT);
  bitSet(HW_ROTENC_PCMSK, HW_ROTENC_BTN_INT);
  bitSet(PCIFR, HW_ROTENC_PCIFR);
  bitSet(PCICR, HW_ROTENC_PCICR);
  interrupts();
}

/// @brief Set rotary encoder's counter value and range
///
/// @param counterValue Counter value to set
/// @param minLimit Minimum limit for counter range
/// @param maxLimit Maximum limit for counter range
/// @param wrap If true, the counter will "wrap around", e.i. if incremented beyond max limit
/// (decremented beyond min limit) it will jump to opposite limit; if false, the counter
/// incremented beyond max limit (decremented beyond min limin) will stay at the same limit
/// @return True if parameters were set successfully or false if there was an error and
/// parameters were not set
///
bool RotEnc::setCounter (int16_t counterValue, int16_t minLimit, int16_t maxLimit, bool wrap) {
  if (minLimit >= maxLimit) return (false);
  if (minLimit < minLimitRange) minLimit = minLimitRange;
  if (maxLimit > maxLimitRange) maxLimit = maxLimitRange;
  if (counterValue > maxLimit) counterValue = maxLimit;
  if (counterValue < minLimit) counterValue = minLimit;
  noInterrupts();
  counter = counterValue * HW_ROTENC_CYCLES_PER_DETENT;
  counterMinLimit = minLimit * HW_ROTENC_CYCLES_PER_DETENT;
  counterMaxLimit = maxLimit * HW_ROTENC_CYCLES_PER_DETENT;
  counterWrap = wrap;
  interrupts();
  return (true);
}
