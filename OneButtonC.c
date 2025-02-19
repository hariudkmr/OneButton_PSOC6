/**
 * This is a port of the OneButton library to C to be used with 
 * the STM32 HAL framework.
 * 
 * @file OneButtonC.h
 *
 * @brief Library for detecting button clicks, doubleclicks and long press
 * pattern on a single button.
 *
 * Original C++ Arduino code License and information given below. This code
 * is released under the same license.
 * @author Matthias Hertel, https://www.mathertel.de
 * @Copyright Copyright (c) by Matthias Hertel, https://www.mathertel.de.
 *                          Ihor Nehrutsa, Ihor.Nehrutsa@gmail.com
 *
 * This work is licensed under a BSD style license. See
 * http://www.mathertel.de/License.aspx
 *
 * More information on: https://www.mathertel.de/Arduino/OneButtonLibrary.aspx
 *
 * Changelog: see OneButtonC.h
 */

#include "OneButtonC.h"
#include <stdlib.h>
#include <string.h>

/*

SETUP:
OneButton_t button1;
OB_Init(&button1);
OB_Setup(&button1, GPIOA, GPIO_PIN_5, true);

IN MAIN:
OB_Tick(&button1);

*/

// ----- Initialization and Default Values -----

void OB_Init(OneButton_t* btn) {
    btn->port = NULL;
    btn->pin = INVALID_PIN;

    btn->debounce_ms = 50;
    btn->click_ms = 400;
    btn->press_ms = 800;
    btn->idle_ms = 1000;

    btn->state = OCS_INIT;
    btn->idleState = false;

    btn->buttonPressedLevel = 0;
    btn->debouncedLevel = false;
    btn->lastDebounceLevel = false;
    
    btn->lastDebounceTime = 0;
    btn->startTime = 0;
    btn->now = 0;

    btn->nClicks = 0;
    btn->maxClicks = 1;

    btn->longPressIntervalMs = 0;
    btn->lastDuringLongPressTime = 0;

    btn->pressFunc = NULL;
    btn->clickFunc = NULL;
    btn->doubleClickFunc = NULL;
    btn->multiClickFunc = NULL;
    btn->longPressStartFunc = NULL;
    btn->longPressStopFunc = NULL;
    btn->duringLongPressFunc = NULL;
    btn->idleFunc = NULL;
}

void OB_Setup(OneButton_t* btn, GPIO_TypeDef* port, uint16_t pin, bool activeLow) {
    btn->port = port;
    btn->pin = pin;
    btn->buttonPressedLevel = activeLow ? 0 : 1;
}

bool OB_Debounce(OneButton_t* btn, bool newLevel) {
    btn->now = HAL_GetTick();  // current (relative) time in msecs.

    if (newLevel && btn->debounce_ms < 0)
        btn->debouncedLevel = newLevel;

    if (btn->lastDebounceLevel == newLevel) {
        if (btn->now - btn->lastDebounceTime >= abs(btn->debounce_ms))
            btn->debouncedLevel = newLevel;
    } else {
        btn->lastDebounceTime = btn->now;
        btn->lastDebounceLevel = newLevel;
    }
    return btn->debouncedLevel;
}

void OB_Tick(OneButton_t* btn) {
    if (btn->pin != INVALID_PIN) {
        bool rawLevel = (HAL_GPIO_ReadPin(btn->port, btn->pin) == btn->buttonPressedLevel);
        OB_FSM(btn, OB_Debounce(btn, rawLevel == btn->buttonPressedLevel));
    }
}

void OB_SetDebounceMs(OneButton_t* btn, int16_t ms) {
    btn->debounce_ms = ms;
}

void OB_SetClickMs(OneButton_t* btn, uint16_t ms) {
    btn->click_ms = ms;
}

void OB_SetPressMs(OneButton_t* btn, uint16_t ms) {
    btn->press_ms = ms;
}

void OB_SetIdleMs(OneButton_t* btn, uint16_t ms) {
    btn->idle_ms = ms;
}

void OB_SetLongPressIntervalMs(OneButton_t* btn, uint16_t ms) {
    btn->longPressIntervalMs = ms;
}

void OB_Reset(OneButton_t* btn) {
    btn->state = OCS_INIT;
    btn->nClicks = 0;
    btn->startTime = HAL_GetTick();
    btn->idleState = false;
}

uint16_t OB_GetNumberClicks(const OneButton_t* btn) {
    return btn->nClicks;
}

bool OB_IsIdle(const OneButton_t* btn) {
    return btn->state == OCS_INIT;
}

bool OB_IsLongPressed(const OneButton_t* btn) {
    return btn->state == OCS_PRESS;
}

void OB_AttachPress(OneButton_t* btn, OneButtonCallback cb) {
    btn->pressFunc = cb;
}

void OB_AttachClick(OneButton_t* btn, OneButtonCallback cb) {
    btn->clickFunc = cb;
}

void OB_AttachDoubleClick(OneButton_t* btn, OneButtonCallback cb) {
    btn->doubleClickFunc = cb;
    btn->maxClicks = btn->maxClicks > 2 ? btn->maxClicks : 2;
}

void OB_AttachMultiClick(OneButton_t* btn, OneButtonCallback cb) {
    btn->multiClickFunc = cb;
    btn->maxClicks = btn->maxClicks > 100 ? btn->maxClicks : 100;
}

void OB_AttachLongPressStart(OneButton_t* btn, OneButtonCallback cb) {
    btn->longPressStartFunc = cb;
}

void OB_AttachLongPressStop(OneButton_t* btn, OneButtonCallback cb) {
    btn->longPressStopFunc = cb;
}

void OB_AttachDuringLongPress(OneButton_t* btn, OneButtonCallback cb) {
    btn->duringLongPressFunc = cb;
}

void OB_AttachIdle(OneButton_t* btn, OneButtonCallback cb) {
    btn->idleFunc = cb;
}

uint16_t OB_GetPressedMs(const OneButton_t* btn) {
    return (btn->now - btn->startTime);
}

uint16_t OB_GetPin(const OneButton_t* btn) {
    return btn->pin;
}

OneButtonState OB_GetState(const OneButton_t* btn) {
    return btn->state;
}

bool OB_GetDebouncedValue(const OneButton_t* btn) {
    return btn->debouncedLevel;
}

void OB_NewState(OneButton_t* btn, OneButtonState nextState) {
    btn->state = nextState;
}

void OB_FSM(OneButton_t* btn, bool activeLevel) {
    uint32_t waitTime = (btn->now - btn->startTime);

    switch (btn->state) {
        case OCS_INIT:
            if (!btn->idleState && (waitTime > btn->idle_ms)) {
                if (btn->idleFunc) {
                    btn->idleState = true;
                    btn->idleFunc();
                }
            }

            if (activeLevel) {
                OB_NewState(btn, OCS_DOWN);
                btn->startTime = btn->now;
                btn->nClicks = 0;

                if (btn->pressFunc) btn->pressFunc();
            }
            break;

        case OCS_DOWN:
            if (!activeLevel) {
                OB_NewState(btn, OCS_UP);
                btn->startTime = btn->now;
            } else if (waitTime > btn->press_ms) {
                if (btn->longPressStartFunc) btn->longPressStartFunc();
                OB_NewState(btn, OCS_PRESS);
            }
            break;

        case OCS_UP:
            btn->nClicks++;
            OB_NewState(btn, OCS_COUNT);
            break;

        case OCS_COUNT:
            if (activeLevel) {
                OB_NewState(btn, OCS_DOWN);
                btn->startTime = btn->now;
            } else if ((waitTime >= btn->click_ms) || (btn->nClicks == btn->maxClicks)) {
                if (btn->nClicks == 1) {
                    if (btn->clickFunc) btn->clickFunc();
                } else if (btn->nClicks == 2) {
                    if (btn->doubleClickFunc) btn->doubleClickFunc();
                } else {
                    if (btn->multiClickFunc) btn->multiClickFunc();
                }
                OB_Reset(btn);
            }
            break;

        case OCS_PRESS:
            if (!activeLevel) {
                OB_NewState(btn, OCS_PRESSEND);
            } else {
                if ((btn->now - btn->lastDuringLongPressTime) >= btn->longPressIntervalMs) {
                    if (btn->duringLongPressFunc) btn->duringLongPressFunc();
                    btn->lastDuringLongPressTime = btn->now;
                }
            }
            break;

        case OCS_PRESSEND:
            if (btn->longPressStopFunc) btn->longPressStopFunc();
            OB_Reset(btn);
            break;

        default:
            OB_NewState(btn, OCS_INIT);
            break;
    }
}