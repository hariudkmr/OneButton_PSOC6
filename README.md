## Note: this library is a WIP, readme below is in DRAFT, some functions might now work.
- Best attempt to port over all OneButton functionality. Currently there is no ability for lamda or parameters in the callback functions as provided by OneButton.

# STM32 OneButton Library

This STM32 library improves the usage of a single button for input.
It shows how to use a digital input pin with a single pushbutton attached
for detecting some of the typical button press events like single clicks, double clicks, and long-time pressing.
This enables you to reuse the same button for multiple functions and lowers the hardware investments.

This is also a sample for implementing simple finite-state machines by using the simple pattern above.

This code is a C port of the C++ Arduino library written by Matthias Hertel at
<https://github.com/mathertel/OneButton>

You can find more details on this library at
<http://www.mathertel.de/Arduino/OneButtonLibrary.aspx>

## Basic STM32 Usage Example

```c
OneButton_t button1;  // Declare global instance
uint16_t press_cnt = 0;

OB_Init(&button1); // Init the button with default params
OB_Setup(&button1, BTN_1_GPIO_Port, BTN_1_Pin, true); // Configure the button

void B1_press() { // Basic function to count button presses
    press_cnt++;
}

OB_AttachPress(&button1, B1_press); // Attach the B1_press function to the Press action

while (1) {
    OB_Tick(&button1); // Call ticks in main loop
}
```

## Getting Started

Clone this repository into your STM32 project or copy the source files into your project directory. Include the header file in your code:

```c
#include "OneButtonC.h"
```

Each physical button requires its own `OneButton_t` instance.

### Initialize Button Instance

You must first declare a global instance and then initialize it:

```c
OneButton_t button1; // Declare global instance

// In your initialization code:
OB_Init(&button1); // Init with default parameters
OB_Setup(&button1, BTN_GPIO_Port, BTN_Pin, true); // Configure pin and active state
```

The `OB_Setup` function takes the following parameters:
- Button instance pointer
- GPIO Port or Port Alias
- GPIO Pin or Pin Alias
- Active state (true for active LOW, false for active HIGH)

### Attach State Events

Once you have your button initialized, you can handle events by attaching them to the button instance.

```c
void B1_press() { // Basic function to count button presses
    press_cnt++;
}

OB_AttachPress(&button1, B1_press); // Attach the B1_press function to the Press action
```

### Don't forget to `OB_Tick()`

In order for `STM32_OneButton` to work correctly, you must call `OB_Tick()` on __each button instance__ within your main loop. If you're not getting any button events, this is probably why.

```c
while (1) {
    OB_Tick(&button1); // Call ticks in main loop
    // Do other things...
}
```

### HAL Function

Note that the 'OB_Tick()' function uses the `HAL_GPIO_ReadPin( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)` call to get the logic level of the pin and returns RESET(0) or SET(1) for the low and high states respectively. 

`HAL_GetTick()` is also called which requires that the SysTick timer be enabled and configured.

## State Events

Here's a full list of events handled by this library:

| Attach Function         | Description                                                   |
| ----------------------- | ------------------------------------------------------------- |
| `OB_AttachPress`        | Fires as soon as a press is detected.                         |
| `OB_AttachClick`        | Fires as soon as a single click press and release is detected.|
| `OB_AttachDoubleClick`  | Fires as soon as a double click is detected.                  |
| `OB_AttachMultiClick`   | Fires as soon as multiple clicks have been detected.          |
| `OB_AttachLongPressStart` | Fires as soon as the button is held down for 800 milliseconds.|
| `OB_AttachDuringLongPress` | Fires periodically as long as the button is held down.        |
| `OB_AttachLongPressStop` | Fires when the button is released after a long hold.          |

### Event Timing

Valid events occur when `OB_Tick()` is called after a specified number of milliseconds. You can use the following functions to change the timing.

__Note:__ Attaching a double click will increase the delay for detecting a single click. If a double click event is not attached, the library will assume a valid single click after one click duration, otherwise it must wait for the double click timeout to pass. This is because a single click callback must not be triggered in case of a double click event.

| Function                | Default    | Description                                                   |
| ----------------------- | ---------- | ------------------------------------------------------------- |
| `OB_SetDebounceMs(int)` | `50 msec`  | Period of time in which to ignore additional level changes.   |
| `OB_SetClickMs(int)`    | `400 msec` | Timeout used to distinguish single clicks from double clicks. |
| `OB_SetPressMs(int)`    | `800 msec` | Duration to hold a button to trigger a long press.            |

You may change these default values but be aware that when you specify too short times it is hard to click twice or you will create a long press instead of a click.

Set debounce ms to a negative value to only debounce on release. `OB_SetDebounceMs(-25);` will immediately update to a pressed state, and will debounce for 25ms going into the released state. This will expedite the `OB_AttachPress` callback function to run instantly.

### Additional Functions

`STM32_OneButton` also provides a couple additional functions to use for querying button status:

| Function                | Description                                                                    |
| ----------------------- | ------------------------------------------------------------------------------ |
| `bool OB_IsLongPressed(const OneButton_t* btn)` | Detect whether or not the button is currently inside a long press.             |
| `uint16_t OB_GetPressedMs(const OneButton_t* btn)` | Get the current number of milliseconds that the button has been held down for. |
| `uint16_t OB_GetPin(const OneButton_t* btn)` | Get the button pin                                                          |
| `OneButtonState OB_GetState(const OneButton_t* btn)` | Get the button state                                                        |
| `bool OB_GetDebouncedValue(const OneButton_t* btn)` | Get the debounced button value                                              |

### `OB_Reset()`

If you wish to reset the internal state of your buttons, call `OB_Reset()`.

## Troubleshooting

If your buttons aren't acting the way they should, check these items:

1. Check your wiring and pin numbers.
2. Did you call `OB_Tick()` on each button instance in your loop?
3. Did you alter your clock timers in any way without adjusting ticks?
4. Are the pins or pin aliases defined correctly?
5. Is the active level configured correctly?
```
