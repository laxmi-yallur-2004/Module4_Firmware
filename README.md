# Module 4 Firmware

## Robust Button Handling, GPIO Output Safety and Interrupt-Driven Input Capture

This module demonstrates three important embedded firmware concepts using an **Arduino UNO** and a **16x2 LCD Keypad Shield**.

### Tasks

1. Robust Button Handling
2. GPIO Output Safety
3. Interrupt-Driven Input Capture

---

## Hardware Used

* Arduino UNO
* 16x2 LCD Keypad Shield
* USB cable
* Jumper wire

---

## LCD Connections

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

---

# 1. Robust Button Handling

## Objective

This task makes the SELECT button reliable.

A mechanical button can produce multiple unwanted signals when pressed. This is called **button bounce**.

The program handles:

* Debouncing
* Short press
* Long press
* Repeat press
* Stuck button

### Button Pin

The SELECT button of the LCD Keypad Shield is connected to:

```text
A0
```

The program reads the button using:

```cpp
analogRead(A0);
```

### Button Timing

| Function        |    Time |
| --------------- | ------: |
| Debounce        |   30 ms |
| Long press      | 1000 ms |
| Repeat          |  500 ms |
| Stuck detection | 5000 ms |

### Working

```text
Button Released
       ↓
Debouncing
       ↓
Button Pressed
       ↓
Short / Long Press
       ↓
Repeat
       ↓
Stuck Detection
```

### Short Press

Press and release the SELECT button quickly.

Expected:

```text
SHORT PRESS
Detected
```

### Long Press

Hold SELECT for about 1 second.

Expected:

```text
LONG PRESS
Detected
```

### Repeat

Continue holding the button.

The program generates repeat events approximately every 500 ms.

Example:

```text
REPEAT:1
Holding...
```

### Stuck Button

If the button remains pressed for about 5 seconds:

```text
BUTTON
STUCK
```

After releasing the button:

```text
STUCK CLEARED
Button released
```

### Why It Is Important

Robust button handling is useful in:

* Keypads
* Control panels
* Industrial devices
* Embedded user interfaces

---

# 2. GPIO Output Safety

## Objective

This task demonstrates how to safely control a GPIO output.

The output is kept **LOW during startup** so that connected hardware does not accidentally turn ON.

### Pins Used

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

### State Machine

The program uses these states:

```text
BOOT
  ↓
INITIALIZING
  ↓
READY
  ↓
ENABLED
```

### BOOT

When the Arduino starts:

```text
Output = LOW
```

This is the safe state.

### INITIALIZING

The firmware initializes the system while keeping the output OFF.

### READY

The system is ready for the SELECT button.

```text
SYSTEM READY
OUTPUT OFF
```

### ENABLED

Press SELECT.

The output becomes HIGH:

```text
OUTPUT
ENABLED
```

D13 is now HIGH.

### Disable

Press SELECT again.

The output becomes LOW:

```text
OUTPUT
DISABLED
```

The system returns to READY.

### State Flow

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

### Why GPIO Safety Is Important

GPIO outputs may control:

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

This task demonstrates how an interrupt can be used to capture a digital signal and measure:

* Frequency
* Period
* Pulse width

The Arduino generates a test signal itself, so an external signal generator is not required.

---

## Connections

Connect:

```text
Arduino D12  ──────────>  Arduino D2
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
| Test Signal Output | D12         |
| SELECT Button      | A0          |

Use one jumper wire:

```text
D12 → D2
```

---

## Test Signal

The program generates a square wave on D12.

The signal is approximately:

```text
HIGH = 500 us
LOW  = 500 us
```

Therefore:

```text
Period = 500 + 500
       = 1000 us
```

Expected frequency:

```text
Frequency ≈ 1000 Hz
```

---

## Interrupt

D2 is used as the interrupt input.

The interrupt detects both:

```text
Rising Edge
Falling Edge
```

### Rising Edge

```text
LOW → HIGH
```

The firmware records the time.

### Falling Edge

```text
HIGH → LOW
```

The firmware calculates the HIGH pulse width.

---

## Period Measurement

The time between two rising edges gives the signal period.

Example:

```text
Rising Edge 1 = 1000 us
Rising Edge 2 = 2000 us

Period = 1000 us
```

---

## Pulse Width Measurement

The time between a rising edge and falling edge gives the HIGH pulse width.

Example:

```text
Rising Edge  = 1000 us
Falling Edge = 1500 us

Pulse Width = 500 us
```

---

## Frequency Calculation

The frequency is calculated using:

```text
Frequency = 1,000,000 / Period_us
```

For a 1000 us period:

```text
Frequency = 1,000,000 / 1000
          = 1000 Hz
```

---

## ISR

ISR means:

**Interrupt Service Routine**

When a signal edge occurs, the ISR runs automatically.

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

The main program performs calculations and updates the LCD.

---

## `volatile`

Variables shared between the ISR and the main program are declared using:

```cpp
volatile
```

Example:

```cpp
volatile unsigned long periodTime;
volatile unsigned long pulseWidth;
volatile bool measurementReady;
```

This tells the compiler that these values can change inside an interrupt.

---

## Expected Output

For the generated test signal, the LCD should show approximately:

```text
FREQ:1000Hz
WIDTH:500us
```

Small differences can occur because the Arduino is generating and measuring the signal using software timing.

---

# Testing

## Test 1 – Robust Button

1. Connect the LCD Keypad Shield.
2. Upload the robust button program.
3. Press SELECT quickly.
4. Check the short-press result.
5. Hold SELECT for more than 1 second.
6. Check the long-press result.
7. Continue holding the button.
8. Check repeat events.
9. Hold for about 5 seconds.
10. Check stuck-button detection.
11. Release the button.
12. Check that the stuck condition clears.

---

## Test 2 – GPIO Output Safety

1. Connect the LCD Keypad Shield.
2. Upload the GPIO output safety program.
3. Check that D13 is LOW during startup.
4. Wait until the system shows READY.
5. Press SELECT.
6. D13 becomes HIGH.
7. Press SELECT again.
8. D13 becomes LOW.
9. The system returns to READY.

---

## Test 3 – Interrupt-Driven Input Capture

1. Connect the LCD Keypad Shield.
2. Connect Arduino D12 to D2.
3. Upload the interrupt input capture program.
4. D12 generates the test signal.
5. D2 receives the signal.
6. The interrupt detects the signal edges.
7. The program measures the period.
8. The program measures the pulse width.
9. The program calculates frequency.
10. The result is displayed on the LCD.

Expected:

```text
FREQ:1000Hz
WIDTH:500us
```

---

# Important Concepts

| Concept       | Meaning                                             |
| ------------- | --------------------------------------------------- |
| GPIO          | General Purpose Input/Output                        |
| ADC           | Analog-to-Digital Converter                         |
| ISR           | Interrupt Service Routine                           |
| IRQ           | Interrupt Request                                   |
| Debouncing    | Removing unwanted button transitions                |
| State Machine | System operating through defined states             |
| `millis()`    | Time in milliseconds                                |
| `micros()`    | Time in microseconds                                |
| `volatile`    | Variable can change unexpectedly, such as in an ISR |

---

# Design Practices Used

This module demonstrates:

* Safe GPIO initialization
* State-machine based firmware
* Button debouncing
* Short and long press detection
* Stuck-button detection
* Interrupt-based signal capture
* Rising and falling edge detection
* Period measurement
* Pulse-width measurement
* Frequency calculation
* Short ISR design
* `volatile` variables
* Non-blocking timing

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

### 1. Robust Button Handling

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

### 2. GPIO Output Safety

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

### 3. Interrupt-Driven Input Capture

```text
D12 Signal
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
```

---

## Final Result

Module 4 provides practical experience with:

**Robust button handling + GPIO safety + interrupts + timing + signal capture + state machines.**

These are important concepts used in real embedded firmware development.
