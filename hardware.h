/*
* Copyright (C) 2017 Nick Naumenko (https://github.com/nnaumenko)
* All rights reserved
* This software may be modified and distributed under the terms
* of the MIT license. See the LICENSE file for details.
*/

/**
 * @file
 * @brief Hardware setup: pins, interrupts, timings, etc
 */

#ifndef HARDWARE_H
#define HARDWARE_H

// @addtogroup neopixel
/// @{

#define HW_NEOPIXEL_PORT  PORTB  ///< Neopixels' pin output port
#define HW_NEOPIXEL_DIR   DDRB   ///< Neopixels' pin direction port
#define HW_NEOPIXEL_BIT   5      ///< Neopixels' pin in the port (5 corresponds to D13 on Nano/Uno)

#define HW_NEOPIXEL_ROWS  8      ///< Rows in neopixel matrix
#define HW_NEOPIXEL_COLS  4      ///< Columns in neopixel matrix

#define HW_NEOPIXEL_NUMBER (HW_NEOPIXEL_ROWS * HW_NEOPIXEL_COLS)   ///< Total number of neopixels

/// @}
///
/// @defgroup neopixel_timings Neopixel timing macros
/// @ingroup neopixel
/// @brief Timing constraints taken mostly from the WS2812 datasheets
///
/// These timings are chosen to be conservative and avoid problems rather than for maximum throughput
/// @{

#define HW_NEOPIXEL_T1H  900    ///< Width of a 1 bit in ns
#define HW_NEOPIXEL_T1L  600    ///< Width of a 1 bit in ns

#define HW_NEOPIXEL_T0H  400    ///< Width of a 0 bit in ns
#define HW_NEOPIXEL_T0L  900    ///< Width of a 0 bit in ns

#define HW_NEOPIXEL_RES  6000   ///< Width of the low gap between bits to cause a frame to latch (in ns)

#define NS_PER_SEC (1000000000L)                      ///< Nanoseconds per second. Note that this has to be SIGNED since we want to be able to check for negative values of derivatives
#define CYCLES_PER_SEC (F_CPU)                        ///< CPU cycles per second
#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )  ///< CPU nanoseconds per cycle
#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )        ///< Convert nanoseconds to number of CPU cycles

/// @}
///
/// @addtogroup rot_enc_control
/// @{

#define HW_ROTENC_PORT    PIND    ///< Rotary encoder pins' input port
#define HW_ROTENC_DIR     DDRD    ///< Rotary encoder pins' direction port
#define HW_ROTENC_OUT     PORTD   ///< Rotary encoder pins' output port (only to enable pullup)

#define HW_ROTENC_A_BIT     2     ///< Line A bit in the port (2 corresponds to D2 on Nano/Uno)
#define HW_ROTENC_B_BIT     3     ///< Line B bit in the port (3 corresponds to D3 on Nano/Uno)
#define HW_ROTENC_BTN_BIT   4     ///< Rotary encoder button bit in the port (4 corresponds to D4 on Nano/Uno)

#define HW_ROTENC_INTVECT PCINT2_vect   ///< Rotary encoder Pin Change Interrupt vector
#define HW_ROTENC_PCMSK   PCMSK2        ///< Rotary encoder Pin Change Interrupt PCMSK register
#define HW_ROTENC_PCIFR   PCIF2         ///< Rotary encoder Pin Change Interrupt PCIFR bit
#define HW_ROTENC_PCICR   PCIE2         ///< Rotary encoder Pin Change Interrupt PCICR bit

#define HW_ROTENC_A_INT     PCINT18   ///< Line A Pin Change Interrupt
#define HW_ROTENC_B_INT     PCINT19   ///< Line B Pin Change Interrupt
#define HW_ROTENC_BTN_INT   PCINT20   ///< Rotary encoder button Pin Change Interrupt

#define HW_ROTENC_CYCLES_PER_DETENT 4   ///< Full pulse cycles per rotary encoder detent (click), set to 1 if encoder has no detents

#define HW_BUTTON_SHORT_CLICK_MIN_TIME  20    ///< Rotary encoder button short click timings (milliseconds)
#define HW_BUTTON_LONG_CLICK_MIN_TIME   500   ///< Rotary encoder button long click timings (milliseconds)

/// @}


#endif // #ifndef HARDWARE_H
