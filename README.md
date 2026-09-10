# Module 4 Firmware

## Robust Button Handling, GPIO Output Safety and Interrupt-Driven Input Capture

This module demonstrates three important embedded firmware concepts using an **Arduino UNO** and **16x2 LCD Keypad Shield**:

1. Robust Button Handling
2. GPIO Output Safety
3. Interrupt-Driven Input Capture

---

# Hardware Used

* Arduino UNO
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

The SELECT button on the LCD Keypad Shield is connected to:

```text
A0
```

---

# 1. Robust Button Handling

## Objective

The first task implements reliable handling of the SELECT button.

Mechanical buttons can produce unwanted rapid electrical transitions when pressed. This is called **button bounce**.

The firmware handles:

* Debouncing
* Short press
* Long press
* Repeat press
* Stuck-button detection

## Button Pin

```text
SELECT → A0
```

The button is read using:

```cpp
analogRead(A0);
```

## Timing

| Function        |    Time |
| --------------- | ------: |
| Debounce        |   30 ms |
| Long press      | 1000 ms |
| Repeat          |  500 ms |
| Stuck detection | 5000 ms |

## Working

```text
BUTTON RELEASED
       ↓
BUTTON DEBOUNCING
       ↓
BUTTON PRESSED
       ↓
SHORT / LONG PRESS
       ↓
REPEAT
       ↓
STUCK DETECTION
```

## Short Press

Press and release SELECT quickly.

Expected:

```text
SHORT PRESS
Detected
```

## Long Press

Hold SELECT for approximately 1 second.

Expected:

```text
LONG PRESS
Detected
```

## Repeat

Continue holding the button.

The firmware generates repeat events approximately every 500 ms.

Example:

```text
REPEAT:1
REPEAT:2
REPEAT:3
```

## Stuck Button

If SELECT remains pressed for approximately 5 seconds:

```text
BUTTON
STUCK
```

After releasing:

```text
STUCK CLEARED
```

## Why It Is Important

Reliable button handling is useful in:

* Keypads
* Control panels
* Industrial equipment
* Embedded user interfaces

---

# 2. GPIO Output Safety

## Objective

The second task demonstrates safe control of a GPIO output.

The output starts in a known safe state so connected hardware does not accidentally turn ON during startup.

## Pins Used

| Function      | Arduino UNO |
| ------------- | ----------- |
| LCD RS        | D8          |
| LCD EN        | D9          |
| LCD D4        | D4          |
| LCD D5        | D5          |
| LCD D6        | D6          |
| LCD D7        | D7          |
| SELECT Button | A0          |
| GPIO Output   | D13         |

## State Machine

```text
BOOT
  ↓
INITIALIZING
  ↓
READY
  ↓ SELECT
ENABLED
  ↓ SELECT
READY
```

## BOOT

During startup:

```text
D13 = LOW
```

This is the safe state.

## INITIALIZING

The firmware initializes the system while keeping the output OFF.

## READY

The system is ready for the SELECT button.

```text
SYSTEM READY
OUTPUT OFF
```

## ENABLED

Press SELECT.

D13 becomes HIGH.

```text
OUTPUT
ENABLED
```

## DISABLED

Press SELECT again.

D13 becomes LOW.

```text
OUTPUT
DISABLED
```

The system returns to READY.

## Why GPIO Safety Is Important

GPIO outputs can control:

* Motors
* Relays
* Valves
* LEDs
* Actuators
* Power circuits

Therefore, outputs should start in a known safe state.

---

# 3. Interrupt-Driven Input Capture

## Objective

The third task demonstrates how an interrupt can capture a digital signal and measure:

* Pulse width
* Period
* Frequency

The Arduino generates its own test signal, so an external signal generator is not required.

---

## Pin Connections

For the final measurement program:

```text
D3 ───────────── D2
     Jumper
```

### Pin Usage

| Function           | Arduino UNO |
| ------------------ | ----------- |
| LCD RS             | D8          |
| LCD EN             | D9          |
| LCD D4             | D4          |
| LCD D5             | D5          |
| LCD D6             | D6          |
| LCD D7             | D7          |
| Interrupt Input    | D2          |
| Test Signal Output | D3          |

**Important:** Connect only one jumper:

```text
D3 → D2
```

---

# Test Signal

The program generates a square wave on D3.

The signal is approximately:

```text
HIGH = 500 us
LOW  = 500 us
```

Therefore:

```text
Period = 500 us + 500 us
       = 1000 us
```

Frequency:

```text
Frequency = 1,000,000 / Period
```

Therefore:

```text
Frequency = 1,000,000 / 1000
          = 1000 Hz
```

Expected pulse width:

```text
500 us
```

---

# Interrupt

D2 is used as the interrupt input.

The Arduino UNO detects signal changes on D2.

The interrupt is configured using:

```cpp
attachInterrupt(
  digitalPinToInterrupt(INPUT_PIN),
  measure,
  CHANGE
);
```

`CHANGE` means the interrupt occurs on both:

```text
LOW → HIGH
```

and:

```text
HIGH → LOW
```

---

# Rising Edge

A rising edge occurs when the signal changes:

```text
LOW → HIGH
```

The firmware records the time using:

```cpp
micros();
```

The time between two rising edges is used to calculate the period.

---

# Falling Edge

A falling edge occurs when the signal changes:

```text
HIGH → LOW
```

The firmware calculates the HIGH pulse width.

Example:

```text
Rising edge  = 1000 us
Falling edge = 1500 us

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
       = 1000 us
```

---

# Frequency Calculation

The firmware calculates frequency using:

```text
Frequency = 1,000,000 / Period_us
```

For a 1000 us period:

```text
Frequency = 1,000,000 / 1000
          = 1000 Hz
```

---

# Pulse Width Calculation

Pulse width is the time the signal remains HIGH.

Example:

```text
Rising Edge  = 1000 us
Falling Edge = 1500 us

Pulse Width = 1500 - 1000
            = 500 us
```

---

# ISR

ISR means:

**Interrupt Service Routine**

When a signal edge occurs, the Arduino automatically executes the interrupt function.

```text
Signal changes
      ↓
D2
      ↓
Interrupt
      ↓
ISR
      ↓
Record time
```

The ISR should be kept short.

The main `loop()` performs the display and Serial Monitor operations.

---

# volatile

Variables shared between the interrupt function and the main program are declared using:

```cpp
volatile
```

Example:

```cpp
volatile unsigned long startTime = 0;
volatile unsigned long width = 0;
volatile unsigned long period = 0;
volatile unsigned long lastRise = 0;
```

`volatile` tells the compiler that the value can change unexpectedly, including inside an interrupt.

---

# LCD Output

The measured result is displayed on the 16x2 LCD.

Expected:

```text
FREQ:1000Hz
WIDTH:500us
```

Small differences are normal because the Arduino generates and measures the signal using software timing.

---

# Serial Monitor Output

Open the Arduino Serial Monitor and select:

```text
9600 baud
```

Expected:

```text
Width: 500  Freq: 1000
Width: 500  Freq: 1000
Width: 500  Freq: 1000
```

---

# Testing

## Test 1 – Robust Button

1. Connect the LCD Keypad Shield.
2. Upload the `robust button` program.
3. Press SELECT quickly.
4. Check the short-press result.
5. Hold SELECT for more than 1 second.
6. Check the long-press result.
7. Continue holding the button.
8. Check repeat events.
9. Hold for approximately 5 seconds.
10. Check stuck-button detection.
11. Release SELECT.
12. Check that the stuck condition clears.

---

## Test 2 – GPIO Output Safety

1. Connect the LCD Keypad Shield.
2. Upload the `Gpio ouput safety` program.
3. Check that D13 is LOW during startup.
4. Wait until the system becomes READY.
5. Press SELECT.
6. D13 becomes HIGH.
7. Press SELECT again.
8. D13 becomes LOW.
9. The system returns to READY.

---

## Test 3 – Interrupt-Driven Input Capture

1. Connect the LCD Keypad Shield.
2. Connect **D3 → D2** using one jumper wire.
3. Upload the `interrupt-driven` program.
4. D3 generates the test signal.
5. D2 receives the signal.
6. The interrupt detects rising and falling edges.
7. The program measures the period.
8. The program measures the pulse width.
9. The program calculates frequency.
10. The result is displayed on the LCD.
11. Open Serial Monitor at **9600 baud**.
12. Check the frequency and pulse width.

Expected:

```text
LCD:

FREQ:1000Hz
WIDTH:500us
```

Serial Monitor:

```text
Width: 500  Freq: 1000
```

---

# Important Concepts

| Concept       | Meaning                                              |
| ------------- | ---------------------------------------------------- |
| GPIO          | General Purpose Input/Output                         |
| ADC           | Analog-to-Digital Converter                          |
| ISR           | Interrupt Service Routine                            |
| IRQ           | Interrupt Request                                    |
| Debouncing    | Removing unwanted button transitions                 |
| State Machine | System operating through defined states              |
| `millis()`    | Time in milliseconds                                 |
| `micros()`    | Time in microseconds                                 |
| `volatile`    | Variable that can change outside normal program flow |

---

# Design Practices Used

This module demonstrates:

* Safe GPIO initialization
* State-machine based firmware
* Button debouncing
* Short-press detection
* Long-press detection
* Repeat detection
* Stuck-button detection
* Interrupt-based signal capture
* Rising-edge detection
* Falling-edge detection
* Period measurement
* Pulse-width measurement
* Frequency calculation
* Short ISR design
* `volatile` variables
* LCD output
* Serial Monitor debugging
* Timing measurement

---

# Repository Structure

```text
Module4_Firmware/
│
├── README.md
│
├── robust button/
│
├── Gpio ouput safety/
│
└── interrupt-driven/
```

---

# Module 4 Summary

## 1. Robust Button Handling

```text
Button
  ↓
Debounce
  ↓
Short / Long Press
  ↓
Repeat
  ↓
Stuck Detection
```

## 2. GPIO Output Safety

```text
BOOT
  ↓
INITIALIZING
  ↓
READY
  ↓
ENABLED
```

The output starts LOW and is enabled only when required.

## 3. Interrupt-Driven Input Capture

```text
D3 Signal
   ↓
D2 Interrupt
   ↓
Rising/Falling Edge
   ↓
Time Measurement
   ↓
Period / Pulse Width
   ↓
Frequency
   ↓
LCD + Serial Monitor
```

---

# Final Result

Module 4 provides practical experience with:

**Robust button handling + GPIO safety + state machines + interrupts + timing + signal capture + frequency measurement + pulse-width measurement + LCD/Serial debugging.**

These are important concepts used in real embedded firmware development.
