/*
============================================================
MODULE 4 FIRMWARE
============================================================

TASK 1 : ROBUST BUTTON HANDLING
TASK 2 : GPIO OUTPUT SAFETY
TASK 3 : INTERRUPT-DRIVEN INPUT CAPTURE

BOARD:
Arduino UNO / ATmega328P

LCD:
RS = D8
EN = D9
D4 = D4
D5 = D5
D6 = D6
D7 = D7

SELECT BUTTON:
A0

GPIO OUTPUT:
D13

INTERRUPT INPUT:
D2

TEST SIGNAL OUTPUT:
D3

CONNECTION FOR TASK 3:

D3 ---------------- D2
       JUMPER

No external signal generator required.

No delay()
No dynamic memory
Non-blocking main loop
============================================================
*/


/* =========================================================
   COMMON INCLUDES
   ========================================================= */

#include <Arduino.h>
#include <LiquidCrystal.h>


/* =========================================================
   COMMON LCD
   ========================================================= */

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);


/* =========================================================
   TASK 1 + TASK 2
   ROBUST BUTTON + GPIO SAFETY
   ========================================================= */

const uint8_t BUTTON_PIN = A0;
const uint8_t GPIO_OUTPUT_PIN = 13;


/*
   SELECT button voltage range.

   Typical LCD keypad shields give approximately this
   ADC range for the SELECT button.
*/

const int SELECT_MIN = 600;
const int SELECT_MAX = 800;


/* =========================================================
   BUTTON TIMING
   ========================================================= */

const unsigned long DEBOUNCE_TIME = 30UL;
const unsigned long LONG_PRESS_TIME = 1000UL;
const unsigned long REPEAT_START_TIME = 1500UL;
const unsigned long REPEAT_INTERVAL = 500UL;
const unsigned long STUCK_TIME = 5000UL;


/* =========================================================
   BUTTON STATE MACHINE
   ========================================================= */

enum ButtonState
{
    BUTTON_RELEASED,
    BUTTON_DEBOUNCING,
    BUTTON_PRESSED,
    BUTTON_LONG,
    BUTTON_STUCK
};

ButtonState buttonState = BUTTON_RELEASED;

unsigned long buttonStateStartTime = 0;
unsigned long pressStartTime = 0;
unsigned long lastRepeatTime = 0;

unsigned int repeatCount = 0;


/* =========================================================
   GPIO STATE
   ========================================================= */

enum GpioState
{
    GPIO_READY,
    GPIO_ENABLED
};

GpioState gpioState = GPIO_READY;


/* =========================================================
   TASK 3
   INTERRUPT-DRIVEN INPUT CAPTURE
   ========================================================= */

const uint8_t INPUT_PIN = 2;
const uint8_t TEST_SIGNAL_PIN = 3;


/*
   Variables updated by the ISR.
*/

volatile unsigned long riseTime = 0;
volatile unsigned long pulseWidth = 0;
volatile unsigned long signalPeriod = 0;
volatile bool measurementReady = false;


/*
   Test signal timing.

   HIGH = approximately 500 us
   LOW  = approximately 500 us

   Period = approximately 1000 us
   Frequency = approximately 1000 Hz
*/

const unsigned long HALF_PERIOD_US = 500UL;

unsigned long lastSignalToggle = 0;

bool testSignalState = LOW;


/*
   Serial output timing.
*/

unsigned long lastMeasurementPrint = 0;


/* =========================================================
   LCD HELPER
   ========================================================= */

void showMessage(const char *line1, const char *line2)
{
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(line1);

    lcd.setCursor(0, 1);
    lcd.print(line2);
}


/* =========================================================
   SELECT BUTTON READING
   ========================================================= */

bool isSelectPressed()
{
    int value = analogRead(BUTTON_PIN);

    return (value >= SELECT_MIN &&
            value <= SELECT_MAX);
}


/* =========================================================
   TASK 3 ISR
   ========================================================= */

void measureSignal()
{
    unsigned long currentTime = micros();


    /*
       Rising edge
    */

    if (digitalRead(INPUT_PIN) == HIGH)
    {
        /*
           Calculate period only after the first rising edge.
        */

        if (riseTime != 0)
        {
            signalPeriod = currentTime - riseTime;
            measurementReady = true;
        }

        riseTime = currentTime;
    }


    /*
       Falling edge
    */

    else
    {
        if (riseTime != 0)
        {
            pulseWidth = currentTime - riseTime;
        }
    }
}


/* =========================================================
   TASK 3
   TEST SIGNAL GENERATOR
   ========================================================= */

void updateTestSignal()
{
    unsigned long currentTime = micros();


    /*
       Non-blocking 500 us timing.

       No delayMicroseconds().
    */

    if ((currentTime - lastSignalToggle) >= HALF_PERIOD_US)
    {
        lastSignalToggle = currentTime;

        testSignalState = !testSignalState;

        digitalWrite(
            TEST_SIGNAL_PIN,
            testSignalState
        );
    }
}


/* =========================================================
   TASK 1 + TASK 2
   BUTTON PROCESSING
   ========================================================= */

void updateButton()
{
    unsigned long now = millis();

    bool pressed = isSelectPressed();


    switch (buttonState)
    {
        /* =================================================
           RELEASED
           ================================================= */

        case BUTTON_RELEASED:

            if (pressed)
            {
                buttonState = BUTTON_DEBOUNCING;

                buttonStateStartTime = now;
            }

            break;


        /* =================================================
           DEBOUNCING
           ================================================= */

        case BUTTON_DEBOUNCING:

            if (!pressed)
            {
                buttonState = BUTTON_RELEASED;
            }
            else if ((now - buttonStateStartTime) >=
                     DEBOUNCE_TIME)
            {
                /*
                   Button is now confirmed pressed.
                */

                buttonState = BUTTON_PRESSED;

                pressStartTime = now;

                lastRepeatTime = now;

                repeatCount = 0;


                /*
                   TASK 2:
                   Toggle GPIO immediately after the
                   debounced SELECT press.
                */

                if (gpioState == GPIO_READY)
                {
                    gpioState = GPIO_ENABLED;

                    digitalWrite(
                        GPIO_OUTPUT_PIN,
                        HIGH
                    );

                    showMessage(
                        "OUTPUT",
                        "ENABLED"
                    );

                    Serial.println(
                        "GPIO: OUTPUT ENABLED"
                    );
                }
                else
                {
                    gpioState = GPIO_READY;

                    digitalWrite(
                        GPIO_OUTPUT_PIN,
                        LOW
                    );

                    showMessage(
                        "OUTPUT",
                        "DISABLED"
                    );

                    Serial.println(
                        "GPIO: OUTPUT DISABLED"
                    );
                }
            }

            break;


        /* =================================================
           PRESSED
           ================================================= */

        case BUTTON_PRESSED:

            /*
               Short press:
               button released before 1 second.
            */

            if (!pressed)
            {
                showMessage(
                    "SHORT PRESS",
                    "Detected"
                );

                Serial.println(
                    "SHORT PRESS Detected"
                );

                buttonState = BUTTON_RELEASED;

                buttonStateStartTime = now;
            }


            /*
               Long press.
            */

            else if ((now - pressStartTime) >=
                     LONG_PRESS_TIME)
            {
                buttonState = BUTTON_LONG;

                lastRepeatTime = now;

                showMessage(
                    "LONG PRESS",
                    "Detected"
                );

                Serial.println(
                    "LONG PRESS Detected"
                );
            }

            break;


        /* =================================================
           LONG PRESS
           ================================================= */

        case BUTTON_LONG:

            /*
               Button released.
            */

            if (!pressed)
            {
                showMessage(
                    "LONG PRESS",
                    "Released"
                );

                Serial.println(
                    "LONG PRESS Released"
                );

                buttonState = BUTTON_RELEASED;

                buttonStateStartTime = now;

                repeatCount = 0;
            }


            /*
               Repeat events.
            */

            else if ((now - lastRepeatTime) >=
                     REPEAT_INTERVAL)
            {
                lastRepeatTime = now;

                repeatCount++;


                lcd.clear();

                lcd.setCursor(0, 0);
                lcd.print("REPEAT:");
                lcd.print(repeatCount);

                lcd.setCursor(0, 1);
                lcd.print("Holding...");


                Serial.print("REPEAT:");
                Serial.println(repeatCount);
            }


            /*
               Stuck-button detection.

               Total continuous press time is measured
               from pressStartTime.
            */

            if ((now - pressStartTime) >= STUCK_TIME)
            {
                buttonState = BUTTON_STUCK;

                showMessage(
                    "BUTTON",
                    "STUCK"
                );

                Serial.println(
                    "BUTTON STUCK"
                );
            }

            break;


        /* =================================================
           STUCK BUTTON
           ================================================= */

        case BUTTON_STUCK:

            if (!pressed)
            {
                showMessage(
                    "STUCK CLEARED",
                    "Button released"
                );

                Serial.println(
                    "STUCK CLEARED"
                );

                buttonState = BUTTON_RELEASED;

                buttonStateStartTime = now;

                repeatCount = 0;
            }

            break;
    }
}


/* =========================================================
   TASK 3
   MEASUREMENT DISPLAY
   ========================================================= */

void printMeasurement()
{
    unsigned long now = millis();


    /*
       Print every 500 ms.
    */

    if ((now - lastMeasurementPrint) < 500UL)
    {
        return;
    }

    lastMeasurementPrint = now;


    unsigned long widthCopy;
    unsigned long periodCopy;


    /*
       Safely copy ISR variables.
    */

    noInterrupts();

    widthCopy = pulseWidth;
    periodCopy = signalPeriod;

    interrupts();


    if (periodCopy > 0)
    {
        unsigned long frequency =
            1000000UL / periodCopy;


        Serial.print("Width: ");
        Serial.print(widthCopy);

        Serial.print(" us");

        Serial.print("  Period: ");
        Serial.print(periodCopy);

        Serial.print(" us");

        Serial.print("  Frequency: ");
        Serial.print(frequency);

        Serial.println(" Hz");
    }
}


/* =========================================================
   SETUP
   ========================================================= */

void setup()
{
    /*
       -----------------------------------------------------
       COMMON INITIALIZATION
       -----------------------------------------------------
    */

    lcd.begin(16, 2);

    Serial.begin(9600);


    /*
       -----------------------------------------------------
       TASK 2
       GPIO SAFE INITIALIZATION
       -----------------------------------------------------
    */

    /*
       Set output LOW before enabling output mode.
       This establishes the safe state.
    */

    digitalWrite(GPIO_OUTPUT_PIN, LOW);

    pinMode(GPIO_OUTPUT_PIN, OUTPUT);

    gpioState = GPIO_READY;


    /*
       -----------------------------------------------------
       TASK 3
       INTERRUPT + TEST SIGNAL
       -----------------------------------------------------
    */

    pinMode(INPUT_PIN, INPUT);

    pinMode(TEST_SIGNAL_PIN, OUTPUT);

    digitalWrite(TEST_SIGNAL_PIN, LOW);


    /*
       D2 receives the signal generated by D3.

       Required jumper:

       D3 -> D2
    */

    attachInterrupt(
        digitalPinToInterrupt(INPUT_PIN),
        measureSignal,
        CHANGE
    );


    /*
       Initialize test signal timing.
    */

    lastSignalToggle = micros();


    /*
       -----------------------------------------------------
       STARTUP MESSAGE
       -----------------------------------------------------
    */

    showMessage(
        "MODULE 4",
        "FIRMWARE READY"
    );


    Serial.println();
    Serial.println("==============================");
    Serial.println("       MODULE 4 FIRMWARE");
    Serial.println("==============================");

    Serial.println();
    Serial.println("TASK 1: ROBUST BUTTON");
    Serial.println("SELECT BUTTON: A0");

    Serial.println();
    Serial.println("TASK 2: GPIO SAFETY");
    Serial.println("GPIO OUTPUT: D13");

    Serial.println();
    Serial.println("TASK 3: INTERRUPT CAPTURE");
    Serial.println("INPUT: D2");
    Serial.println("TEST SIGNAL: D3");
    Serial.println("CONNECT D3 -> D2");

    Serial.println();
    Serial.println("SYSTEM READY");
}


/* =========================================================
   LOOP
   ========================================================= */

void loop()
{
    /*
       TASK 3:
       Generate approximately 1 kHz signal.
    */

    updateTestSignal();


    /*
       TASK 1 + TASK 2:
       Process SELECT button.
    */

    updateButton();


    /*
       TASK 3:
       Print measured signal.
    */

    printMeasurement();
}
