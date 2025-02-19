
// -----
// Original code License and information given below. This code is released under
// the same license.:
// OneButton.h - Library for detecting button clicks, doubleclicks and long
// press pattern on a single button. This class is implemented for use with the
// Arduino environment. Copyright (c) by Matthias Hertel,
// http://www.mathertel.de This work is licensed under a BSD style license. See
// http://www.mathertel.de/License.aspx More information on:
// http://www.mathertel.de/Arduino
// -----
// Feb 2025 : V0.1 Port from C++ Arduino library to C
// -----

#ifndef ONEBUTTONC_H
#define ONEBUTTONC_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h" // Include the HAL library appropriate for your MCU

// Define the invalid pin
#define INVALID_PIN UINT16_MAX

// Callback type
typedef void (*OneButtonCallback)(void);

// Finite states
typedef enum {
  OCS_INIT,
  OCS_DOWN,
  OCS_UP,
  OCS_COUNT,
  OCS_PRESS,
  OCS_PRESSEND
} OneButtonState;

// Main struct for OneButton
typedef struct {
  GPIO_TypeDef* port;
  uint16_t pin;
 
  //Intervals
  int16_t debounce_ms;
  uint16_t click_ms;
  uint16_t press_ms;
  uint16_t idle_ms;
  
  // State machine
  OneButtonState state;
  bool idleState;

  //Levels
  uint8_t buttonPressedLevel;
  bool debouncedLevel;
  bool lastDebounceLevel;

  //Timers
  uint32_t lastDebounceTime;
  uint32_t startTime;
  uint32_t now;

  // Click counter
  uint16_t nClicks;
  uint16_t maxClicks;

  // Long press interval
  uint16_t longPressIntervalMs;
  uint32_t lastDuringLongPressTime;

  // Event callbacks
  OneButtonCallback pressFunc;
  OneButtonCallback clickFunc;
  OneButtonCallback doubleClickFunc;
  OneButtonCallback multiClickFunc;
  OneButtonCallback longPressStartFunc;
  OneButtonCallback longPressStopFunc;
  OneButtonCallback duringLongPressFunc;
  OneButtonCallback idleFunc;
} OneButton_t;

// ----- Function prototypes -----

void OB_Init(OneButton_t* btn);
void OB_Setup(OneButton_t* btn, GPIO_TypeDef* port, uint16_t pin, bool activeLow);

bool OB_Debounce(OneButton_t* btn, bool newLevel);
void OB_Tick(OneButton_t* btn /*, bool rawLevel*/);
void OB_Reset(OneButton_t* btn);
uint16_t OB_GetNumberClicks(const OneButton_t* btn);
bool OB_IsIdle(const OneButton_t* btn);
bool OB_IsLongPressed(const OneButton_t* btn);

// Setter functions for intervals
void OB_SetDebounceMs(OneButton_t* btn, int16_t ms);
void OB_SetClickMs(OneButton_t* btn, uint16_t ms);
void OB_SetPressMs(OneButton_t* btn, uint16_t ms);
void OB_SetIdleMs(OneButton_t* btn, uint16_t ms);
void OB_SetLongPressIntervalMs(OneButton_t* btn, uint16_t ms);

// Attach callback function Prototypes
void OB_AttachPress(OneButton_t* btn, OneButtonCallback cb);
void OB_AttachClick(OneButton_t* btn, OneButtonCallback cb);
void OB_AttachDoubleClick(OneButton_t* btn, OneButtonCallback cb);
void OB_AttachMultiClick(OneButton_t* btn, OneButtonCallback cb);
void OB_AttachLongPressStart(OneButton_t* btn, OneButtonCallback cb);
void OB_AttachLongPressStop(OneButton_t* btn, OneButtonCallback cb);
void OB_AttachDuringLongPress(OneButton_t* btn, OneButtonCallback cb);
void OB_AttachIdle(OneButton_t* btn, OneButtonCallback cb);

// Getter functions
uint16_t OB_GetPressedMs(const OneButton_t* btn);
uint16_t OB_GetPin(const OneButton_t* btn);
OneButtonState OB_GetState(const OneButton_t* btn);
bool OB_GetDebouncedValue(const OneButton_t* btn);

// Internal state machine functions
void OB_NewState(OneButton_t* btn, OneButtonState nextState);
void OB_FSM(OneButton_t* btn, bool activeLevel);

#endif // ONEBUTTONC_H