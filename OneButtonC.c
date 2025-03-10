/**
 * This is a port of the OneButton library to C to be used with
 * the PSOC6 PDL framework.
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

// ----- Initialization and Default Values -----

void OB_Init(OneButton_t *btn)
{

    if (!btn) {
        return;
    }

    // Initialize hardware-related values
    btn->port = NULL;
    btn->pin = INVALID_PIN;

    // Set default timing parameters (in milliseconds)
    btn->debounce_ms = DEFAULT_DEBOUNCE_MS;
    btn->click_ms = DEFAULT_CLICK_MS;
    btn->press_ms = DEFAULT_PRESS_MS;
    btn->idle_ms = DEFAULT_IDLE_MS;

    // Initialize state values
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

    // Long press values initialization
    btn->longPressIntervalMs = DEFAULT_LONG_PRESS_INTERVAL_MS;
    btn->lastDuringLongPressTime = 0;

    // Nullify all callback pointers
    btn->pressFunc = NULL;
    btn->clickFunc = NULL;
    btn->doubleClickFunc = NULL;
    btn->multiClickFunc = NULL;
    btn->longPressStartFunc = NULL;
    btn->longPressStopFunc = NULL;
    btn->duringLongPressFunc = NULL;
    btn->idleFunc = NULL;
}

void OB_Setup(OneButton_t *btn, GPIO_PRT_Type *port, uint32_t pin, bool activeLow)
{

    if (!btn) {
        return;
    }

    btn->port = port;
    btn->pin = pin;
    btn->buttonPressedLevel = activeLow ? 0 : 1;
}

bool OB_Debounce(OneButton_t *btn, bool btnActive)
{
    btn->now = Cy_SysTick_GetValue(); // current (relative) time in msecs.

    if (btnActive && btn->debounce_ms < 0)
        btn->debouncedLevel = btnActive;

    if (btn->lastDebounceLevel == btnActive) {
        if (btn->now - btn->lastDebounceTime >= abs(btn->debounce_ms))
            btn->debouncedLevel = btnActive;
    } else {
        btn->lastDebounceTime = btn->now;
        btn->lastDebounceLevel = btnActive;
    }
    return btn->debouncedLevel;
}

void OB_Tick(OneButton_t *btn)
{
    if (btn->pin != INVALID_PIN) {
        bool btnActive = (Cy_GPIO_Read(btn->port, btn->pin) == btn->buttonPressedLevel);
        OB_FSM(btn, OB_Debounce(btn, btnActive));
    }
}

void OB_SetDebounceMs(OneButton_t *btn, int16_t ms)
{
    btn->debounce_ms = ms;
}

void OB_SetClickMs(OneButton_t *btn, uint16_t ms)
{
    btn->click_ms = ms;
}

void OB_SetPressMs(OneButton_t *btn, uint16_t ms)
{
    btn->press_ms = ms;
}

void OB_SetIdleMs(OneButton_t *btn, uint16_t ms)
{
    btn->idle_ms = ms;
}

void OB_SetLongPressIntervalMs(OneButton_t *btn, uint16_t ms)
{
    btn->longPressIntervalMs = ms;
}

void OB_Reset(OneButton_t *btn)
{
    btn->state = OCS_INIT;
    btn->nClicks = 0;
    btn->startTime = Cy_SysTick_GetValue();
    btn->idleState = false;
}

uint16_t OB_GetNumberClicks(const OneButton_t *btn)
{
    return btn->nClicks;
}

bool OB_IsIdle(const OneButton_t *btn)
{
    return btn->state == OCS_INIT;
}

bool OB_IsLongPressed(const OneButton_t *btn)
{
    return btn->state == OCS_PRESS;
}

bool OB_AttachCallback(OneButton_t *btn, OneButtonEvent event, OneButtonCallback cb)
{
    if (!btn) {
        return false;
    }

    switch (event) {
    case OB_EV_PRESS:
        btn->pressFunc = cb;
        break;

    case OB_EV_CLICK:
        btn->clickFunc = cb;
        break;

    case OB_EV_DOUBLE_CLICK:
        btn->doubleClickFunc = cb;
        btn->maxClicks = btn->maxClicks > 2 ? btn->maxClicks : 2;
        break;

    case OB_EV_MULTI_CLICK:
        btn->multiClickFunc = cb;
        btn->maxClicks = btn->maxClicks > 100 ? btn->maxClicks : 100;
        break;

    case OB_EV_LONG_PRESS_START:
        btn->longPressStartFunc = cb;
        break;

    case OB_EV_LONG_PRESS_STOP:
        btn->longPressStopFunc = cb;
        break;

    case OB_EV_DURING_LONG_PRESS:
        btn->duringLongPressFunc = cb;
        break;

    case OB_EV_IDLE:
        btn->idleFunc = cb;
        break;

    default:
        return false; // Invalid event type
    }

    return true;
}

uint16_t OB_GetPressedMs(const OneButton_t *btn)
{
    return (btn->now - btn->startTime);
}

uint16_t OB_GetPin(const OneButton_t *btn)
{
    return btn->pin;
}

OneButtonState OB_GetState(const OneButton_t *btn)
{
    return btn->state;
}

bool OB_GetDebouncedValue(const OneButton_t *btn)
{
    return btn->debouncedLevel;
}

void OB_NewState(OneButton_t *btn, OneButtonState nextState)
{
    btn->state = nextState;
}

void OB_FSM(OneButton_t *btn, bool activeLevel)
{

    if (!btn) {
        return;
    }

    uint32_t waitTime = (btn->now - btn->startTime);

    switch (btn->state) {
    case OCS_INIT: // on idle for idle_ms call idle function
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

            if (btn->pressFunc) {
                btn->pressFunc();
            }
        }
        break;

    case OCS_DOWN: // waiting for level to become inactive.
        if (!activeLevel) {
            OB_NewState(btn, OCS_UP);
            btn->startTime = btn->now;

        } else if (waitTime > btn->press_ms) {
            if (btn->longPressStartFunc) {
                btn->longPressStartFunc();
            }
            OB_NewState(btn, OCS_PRESS);
        }
        break;

    case OCS_UP: // level is inactive
        btn->nClicks++;
        OB_NewState(btn, OCS_COUNT);
        break;

    case OCS_COUNT: // dobounce time is over, count clicks
        if (activeLevel) {
            OB_NewState(btn, OCS_DOWN);
            btn->startTime = btn->now;

        } else if ((waitTime >= btn->click_ms) || (btn->nClicks == btn->maxClicks)) {
            // now we know how many clicks have been made.
            if (btn->nClicks == 1) {
                if (btn->clickFunc) {
                    btn->clickFunc();
                }
            } else if (btn->nClicks == 2) {
                if (btn->doubleClickFunc) {
                    btn->doubleClickFunc();
                }
            } else {
                if (btn->multiClickFunc) {
                    btn->multiClickFunc();
                }
            }
            OB_Reset(btn);
        }
        break;

    case OCS_PRESS: // button was released
        if (!activeLevel) {
            OB_NewState(btn, OCS_PRESSEND);

        } else {
            if ((btn->now - btn->lastDuringLongPressTime) >= btn->longPressIntervalMs) {

                if (btn->duringLongPressFunc) {
                    btn->duringLongPressFunc();
                }
                btn->lastDuringLongPressTime = btn->now;
            }
        }
        break;

    case OCS_PRESSEND: // unknown state detected -> reset state machine
        if (btn->longPressStopFunc) {
            btn->longPressStopFunc();
        }
        OB_Reset(btn);
        break;

    default:
        OB_NewState(btn, OCS_INIT);
        break;
    }
}
