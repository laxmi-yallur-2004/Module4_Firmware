# Module 4 Firmware

## Robust Button Handling, GPIO Output Safety and Interrupt-Driven Input Capture

This project demonstrates three important embedded firmware concepts using an **Arduino UNO** and a **16x2 LCD Keypad Shield**:

1. Robust Button Handling
2. GPIO Output Safety
3. Interrupt-Driven Input Capture

All three tasks are implemented in **one Arduino `.ino` file**.

---

# Project Structure

```text
Module4_Firmware/
│
├── module4_firmware.ino
└── README.md
```

There is only **one firmware source file**.

The common hardware declarations, LCD object, `setup()`, and `loop()` are not duplicated.

---

# Hardware Used

* Arduino UNO
* ATmega328P
* 16x2 LCD Keypad Shield
* USB cable
* Jumper wire

---

# LCD Connections

| LCD Pin | Arduino UNO |
| ------- | ----------- |
| RS      | D8          |
| EN      | D9          |
| D4      | D4          |
| D5      | D5          |
| D6      | D6          |
| D7      | D7          |

LCD initialization:

```cpp
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
```

The LCD Keypad Shield SELECT button is connected to:

```text
A0
```

---

# Pin Summary

| Function           | Arduino UNO Pin |
| ------------------ | --------------- |
| LCD RS             | D8              |
| LCD EN             | D9              |
| LCD D4             | D4              |
| LCD D5             | D5              |
| LCD D6             | D6              |
| LCD D7             | D7              |
| SELECT Button      | A0              |
| GPIO Output        | D13             |
| Interrupt Input    | D2              |
| Test Signal Output | D3              |

---

# Task 1 – Robust Button Handling

## Objective

The first task demonstrates reliable handling of the SELECT button on the LCD Keypad Shield.

Mechanical buttons can produce unwanted rapid electrical transitions when pressed. This is called **button bounce**.

The firmware handles:

* Debouncing
* Short press
* Long press
* Repeat events
* Stuck-button detection
* Recovery after a stuck button is released

---

# Button Input

The SELECT button is read through the analog input:

```cpp
analogRead(A0);
```

The firmware checks whether the measured ADC value is within the SELECT-button range.

---

# Button Timing

| Function               |    Time |
| ---------------------- | ------: |
| Debounce               |   30 ms |
| Long press             | 1000 ms |
| Repeat interval        |  500 ms |
| Stuck-button detection | 5000 ms |

The firmware uses `millis()` for these timings.

It does not use `delay()`.

---

# Button State Machine

The button is handled using defined states:

```text
BUTTON_RELEASED
        ↓
BUTTON_DEBOUNCING
        ↓
BUTTON_PRESSED
        ↓
BUTTON_LONG
        ↓
BUTTON_STUCK
```

The button can return to the released state when the SELECT button is released.

---

# Short Press

A short press occurs when the SELECT button is pressed and released before the long-press time.

Expected Serial Monitor output:

```text
SHORT PRESS Detected
```

LCD:

```text
SHORT PRESS
Detected
```

---

# Long Press

If the SELECT button remains pressed for approximately 1 second, the firmware detects a long press.

Expected Serial Monitor output:

```text
LONG PRESS Detected
```

LCD:

```text
LONG PRESS
Detected
```

---

# Repeat Events

When the button remains pressed, the firmware generates repeat events approximately every 500 ms.

Example:

```text
REPEAT:1
REPEAT:2
REPEAT:3
```

The repeat count is displayed on the LCD.

---

# Stuck-Button Detection

If the SELECT button remains continuously pressed for approximately 5 seconds, the firmware enters the stuck-button state.

Serial Monitor:

```text
BUTTON STUCK
```

LCD:

```text
BUTTON
STUCK
```

After the button is released:

```text
STUCK CLEARED
```

The firmware then returns to the normal released state.

---

# Task 2 – GPIO Output Safety

## Objective

The second task demonstrates how to initialize and control a GPIO output safely.

The GPIO output is:

```text
D13
```

D13 is the built-in LED output on the Arduino UNO.

---

# Safe Startup

The output is placed in a known safe state before normal operation.

During startup:

```text
D13 = LOW
```

This prevents the output from accidentally starting HIGH.

The firmware first writes the safe LOW state and then configures D13 as an output.

---

# GPIO States

The firmware uses two GPIO states:

```text
GPIO_READY
GPIO_ENABLED
```

Initial state:

```text
GPIO_READY
```

D13:

```text
LOW
```

---

# Enabling the Output

When the SELECT button is pressed after debouncing:

```text
D13 → HIGH
```

Serial Monitor:

```text
GPIO: OUTPUT ENABLED
```

LCD:

```text
OUTPUT
ENABLED
```

---

# Disabling the Output

When SELECT is pressed again:

```text
D13 → LOW
```

Serial Monitor:

```text
GPIO: OUTPUT DISABLED
```

LCD:

```text
OUTPUT
DISABLED
```

The system returns to the ready state.

---

# GPIO Safety Flow

```text
BOOT
  ↓
Safe LOW Output
  ↓
GPIO READY
  ↓
SELECT Button
  ↓
GPIO ENABLED
  ↓
SELECT Button
  ↓
GPIO DISABLED
  ↓
GPIO READY
```

---

# Why GPIO Safety Is Important

GPIO outputs can control external hardware such as:

* LEDs
* Relays
* Motors
* Valves
* Actuators
* Power-control circuits

Therefore, an embedded system should initialize important outputs to a known safe state.

---

# Task 3 – Interrupt-Driven Input Capture

## Objective

The third task demonstrates interrupt-driven measurement of a digital signal.

The firmware measures:

* Pulse width
* Period
* Frequency

The Arduino generates its own test signal, so an external signal generator is not required.

---

# Pin Connections

Connect one jumper wire:

```text
D3 ───────── D2
       |
     Jumper
```

D3 generates the test signal.

D2 receives the signal and is configured as the interrupt input.

---

# Task 3 Pin Usage

| Function           | Arduino UNO |
| ------------------ | ----------- |
| Interrupt Input    | D2          |
| Test Signal Output | D3          |

Important:

```text
D3 → D2
```

Only one jumper is required.

---

# Test Signal Generation

The firmware generates a square-wave test signal on D3.

The timing is approximately:

```text
HIGH = 500 microseconds
LOW  = 500 microseconds
```

Therefore:

```text
Period = HIGH + LOW

Period = 500 us + 500 us

Period ≈ 1000 us
```

---

# Frequency

Frequency is calculated using:

```text
Frequency = 1,000,000 / Period_us
```

For an approximately 1000 microsecond period:

```text
Frequency = 1,000,000 / 1000

Frequency ≈ 1000 Hz
```

The actual measured value can have small differences because the signal is generated and measured by software on the Arduino.

---

# Interrupt Configuration

D2 is configured using the external interrupt:

```cpp
attachInterrupt(
    digitalPinToInterrupt(INPUT_PIN),
    measureSignal,
    CHANGE
);
```

`CHANGE` means the interrupt occurs on both signal transitions:

```text
LOW → HIGH
```

and:

```text
HIGH → LOW
```

---

# Rising Edge

A rising edge occurs when:

```text
LOW → HIGH
```

When this happens, the interrupt routine records the current time using:

```cpp
micros();
```

The time between two rising edges is used to determine the signal period.

---

# Falling Edge

A falling edge occurs when:

```text
HIGH → LOW
```

The firmware calculates how long the signal remained HIGH.

This gives the pulse width.

For example:

```text
Rising edge  = 1000 us
Falling edge = 1500 us

Pulse Width = 1500 - 1000

Pulse Width = 500 us
```

---

# Period Measurement

The time between two rising edges gives the period.

Example:

```text
Rising Edge 1 = 1000 us
Rising Edge 2 = 2000 us

Period = 2000 - 1000

Period = 1000 us
```

---

# Frequency Calculation

The firmware calculates frequency using:

```text
Frequency = 1,000,000 / Period_us
```

For example:

```text
Period = 1000 us

Frequency = 1,000,000 / 1000

Frequency = 1000 Hz
```

---

# Interrupt Service Routine

ISR means:

**Interrupt Service Routine**

When the signal changes on D2, the Arduino automatically executes:

```cpp
void measureSignal()
```

The ISR:

1. Reads the current time
2. Detects the signal edge
3. Records the rising-edge time
4. Calculates the period
5. Calculates the pulse width
6. Sets the measurement-ready flag

The ISR is kept short.

The main `loop()` performs the Serial Monitor output.

---

# volatile Variables

Variables shared between the ISR and the main program are declared using `volatile`.

For example:

```cpp
volatile unsigned long riseTime = 0;
volatile unsigned long pulseWidth = 0;
volatile unsigned long signalPeriod = 0;
volatile bool measurementReady = false;
```

`volatile` tells the compiler that these values can change outside the normal program flow, including inside an interrupt.

---

# Safe Access to ISR Data

The main program copies ISR-generated timing values using:

```cpp
noInterrupts();
```

followed by:

```cpp
interrupts();
```

This prevents the values from being changed while they are being copied.

---

# Non-Blocking Design

The firmware does not use:

```cpp
delay();
```

The test signal is generated using:

```cpp
micros();
```

and the button handling uses:

```cpp
millis();
```

This allows all three tasks to operate inside the same main loop without blocking the processor.

---

# LCD Output

The LCD is used for button and GPIO status.

Examples:

```text
OUTPUT
ENABLED
```

```text
OUTPUT
DISABLED
```

```text
SHORT PRESS
Detected
```

```text
LONG PRESS
Detected
```

```text
BUTTON
STUCK
```

The interrupt measurement is reported through the Serial Monitor.

---

# Serial Monitor

Open the Arduino Serial Monitor at:

```text
9600 baud
```

At startup:

```text
==============================
       MODULE 4 FIRMWARE
==============================

TASK 1: ROBUST BUTTON
SELECT BUTTON: A0

TASK 2: GPIO SAFETY
GPIO OUTPUT: D13

TASK 3: INTERRUPT CAPTURE
INPUT: D2
TEST SIGNAL: D3
CONNECT D3 -> D2

SYSTEM READY
```

During Task 3, measurements are printed periodically.

Example:

```text
Width: 500 us  Period: 1000 us  Frequency: 1000 Hz
```

Small differences from the theoretical values are possible because the signal is generated and measured using software timing.

---

# Testing

## Test 1 – Robust Button Handling

1. Connect the LCD Keypad Shield.
2. Upload `module4_firmware.ino`.
3. Open Serial Monitor at 9600 baud.
4. Press and release SELECT quickly.
5. Check:

```text
SHORT PRESS Detected
```

6. Hold SELECT for approximately 1 second.
7. Check:

```text
LONG PRESS Detected
```

8. Continue holding the button.
9. Check repeat events:

```text
REPEAT:1
REPEAT:2
REPEAT:3
```

10. Continue holding for approximately 5 seconds.
11. Check:

```text
BUTTON STUCK
```

12. Release SELECT.
13. Check:

```text
STUCK CLEARED
```

---

# Test 2 – GPIO Output Safety

1. Keep the LCD Keypad Shield connected.
2. Upload `module4_firmware.ino`.
3. Check that D13 starts LOW.
4. Press SELECT.
5. Check:

```text
GPIO: OUTPUT ENABLED
```

6. D13 becomes HIGH.
7. Press SELECT again.
8. Check:

```text
GPIO: OUTPUT DISABLED
```

9. D13 becomes LOW again.

The GPIO therefore starts from a known safe state and changes state only after a valid button event.

---

# Test 3 – Interrupt-Driven Input Capture

1. Connect the LCD Keypad Shield.
2. Connect:

```text
D3 → D2
```

using one jumper wire.

3. Upload `module4_firmware.ino`.
4. Open Serial Monitor.
5. Select:

```text
9600 baud
```

6. D3 generates the test signal.
7. D2 receives the signal.
8. The interrupt detects both rising and falling edges.
9. The firmware measures pulse width and period.
10. Frequency is calculated from the measured period.

Expected values are approximately:

```text
Width ≈ 500 us
Period ≈ 1000 us
Frequency ≈ 1000 Hz
```

---

# Complete System Flow

```text
                 MODULE 4
                    |
        +-----------+-----------+
        |           |           |
        ↓           ↓           ↓
     TASK 1      TASK 2      TASK 3
        |           |           |
     Button       GPIO       Interrupt
        |           |           |
      A0           D13          D2
        |           |           |
   State Machine   Safe      Signal Capture
        |          Output         |
        |           |            |
        +-----------+------------+
                    |
                Main loop()
```

---

# Important Embedded Concepts

| Concept           | Meaning                                                     |
| ----------------- | ----------------------------------------------------------- |
| GPIO              | General Purpose Input/Output                                |
| ADC               | Analog-to-Digital Converter                                 |
| ISR               | Interrupt Service Routine                                   |
| IRQ               | Interrupt Request                                           |
| Debouncing        | Removing unwanted button transitions                        |
| State Machine     | Firmware operating through defined states                   |
| `millis()`        | Measures time in milliseconds                               |
| `micros()`        | Measures time in microseconds                               |
| `volatile`        | Indicates a variable can change outside normal program flow |
| Interrupt Capture | Measuring signal events using interrupts                    |
| Pulse Width       | Time for which a signal remains HIGH                        |
| Period            | Time between repeated signal edges                          |
| Frequency         | Number of cycles per second                                 |

---

# Design Practices Used

This module demonstrates:

* Single-file firmware organization
* One common LCD declaration
* One `setup()`
* One `loop()`
* Safe GPIO initialization
* State-machine-based button handling
* Button debouncing
* Short-press detection
* Long-press detection
* Repeat detection
* Stuck-button detection
* Interrupt-driven signal capture
* Rising-edge detection
* Falling-edge detection
* Period measurement
* Pulse-width measurement
* Frequency calculation
* Short ISR design
* `volatile` shared variables
* Safe access to ISR data
* Non-blocking timing
* LCD status output
* Serial Monitor debugging

---

# Why This Module Is Important

This module combines several practical embedded-firmware techniques in one program.

### Task 1

```text
Button
   ↓
Debounce
   ↓
State Machine
   ↓
Short / Long / Repeat
   ↓
Stuck Detection
```

### Task 2

```text
Startup
   ↓
Safe GPIO State
   ↓
READY
   ↓
SELECT
   ↓
D13 HIGH
   ↓
SELECT
   ↓
D13 LOW
```

### Task 3

```text
D3 Test Signal
      ↓
      D2
      ↓
Interrupt
      ↓
Rising / Falling Edge
      ↓
Time Measurement
      ↓
Pulse Width + Period
      ↓
Frequency
      ↓
Serial Monitor
```

---

# Final Result

Module 4 provides practical experience with:

**Robust button handling + GPIO safety + state machines + interrupts + timing + signal capture + frequency measurement + pulse-width measurement.**

All three tasks are integrated into **one `module4_firmware.ino` file** without duplicating common declarations such as the LCD object, `setup()`, and `loop()`.

The firmware uses a non-blocking design so the different functions can operate together in the same application.
