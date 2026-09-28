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

#include <Arduino.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

/* TASK 1 + TASK 2 */
const byte BUTTON_PIN = A0;
const byte GPIO_PIN = 13;

const int SELECT_MIN = 600;
const int SELECT_MAX = 800;

const unsigned long DEBOUNCE_TIME = 30UL;
const unsigned long LONG_PRESS_TIME = 1000UL;
const unsigned long REPEAT_INTERVAL = 500UL;
const unsigned long STUCK_TIME = 5000UL;

/* BUTTON STATES */
enum ButtonState
{
    BUTTON_RELEASED,
    BUTTON_PRESS_DEBOUNCE,
    BUTTON_PRESSED,
    BUTTON_LONG,
    BUTTON_RELEASE_DEBOUNCE,
    BUTTON_STUCK
};

ButtonState buttonState = BUTTON_RELEASED;

unsigned long buttonStateStartTime = 0;
unsigned long pressStartTime = 0;
unsigned long lastRepeatTime = 0;

unsigned int repeatCount = 0;

bool wasLongPress = false;

/* GPIO STATE */
enum GpioState
{
    GPIO_DISABLED,
    GPIO_ENABLED
};

GpioState gpioState = GPIO_DISABLED;

/* TASK 3 */
const byte INPUT_PIN = 2;
const byte TEST_SIGNAL_PIN = 3;

volatile unsigned long riseTime = 0;
volatile unsigned long pulseWidth = 0;
volatile unsigned long signalPeriod = 0;

const unsigned long HALF_PERIOD_US = 500UL;

unsigned long lastSignalToggle = 0;
bool testSignalState = LOW;

unsigned long lastMeasurementPrint = 0;

/* LCD */
void showMessage(const char *line1, const char *line2)
{
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(line1);

    lcd.setCursor(0, 1);
    lcd.print(line2);
}

/* BUTTON READ */
bool isSelectPressed()
{
    int value = analogRead(BUTTON_PIN);

    return value >= SELECT_MIN &&
           value <= SELECT_MAX;
}

/* TASK 3 INTERRUPT */
void measureSignal()
{
    unsigned long now = micros();

    if (digitalRead(INPUT_PIN) == HIGH)
    {
        if (riseTime != 0)
        {
            signalPeriod = now - riseTime;
        }

        riseTime = now;
    }
    else
    {
        if (riseTime != 0)
        {
            pulseWidth = now - riseTime;
        }
    }
}

/* TASK 3 SIGNAL GENERATOR */
void updateTestSignal()
{
    unsigned long now = micros();

    if ((now - lastSignalToggle) >= HALF_PERIOD_US)
    {
        lastSignalToggle = now;

        testSignalState = !testSignalState;

        digitalWrite(TEST_SIGNAL_PIN, testSignalState);
    }
}

/* TASK 1 + TASK 2 */
void updateButton()
{
    unsigned long now = millis();
    bool pressed = isSelectPressed();

    switch (buttonState)
    {
        case BUTTON_RELEASED:

            if (pressed)
            {
                buttonState = BUTTON_PRESS_DEBOUNCE;
                buttonStateStartTime = now;
            }

            break;


        case BUTTON_PRESS_DEBOUNCE:

            if (!pressed)
            {
                buttonState = BUTTON_RELEASED;
            }
            else if ((now - buttonStateStartTime) >= DEBOUNCE_TIME)
            {
                buttonState = BUTTON_PRESSED;

                pressStartTime = now;
                lastRepeatTime = now;
                repeatCount = 0;
                wasLongPress = false;

                if (gpioState == GPIO_DISABLED)
                {
                    gpioState = GPIO_ENABLED;

                    digitalWrite(GPIO_PIN, HIGH);

                    showMessage("OUTPUT", "ENABLED");

                    Serial.println("GPIO: OUTPUT ENABLED");
                }
                else
                {
                    gpioState = GPIO_DISABLED;

                    digitalWrite(GPIO_PIN, LOW);

                    showMessage("OUTPUT", "DISABLED");

                    Serial.println("GPIO: OUTPUT DISABLED");
                }
            }

            break;


        case BUTTON_PRESSED:

            if (!pressed)
            {
                buttonState = BUTTON_RELEASE_DEBOUNCE;
                buttonStateStartTime = now;
            }
            else if ((now - pressStartTime) >= LONG_PRESS_TIME)
            {
                buttonState = BUTTON_LONG;
                lastRepeatTime = now;
                wasLongPress = true;

                showMessage("LONG PRESS", "Detected");

                Serial.println("LONG PRESS Detected");
            }

            break;


        case BUTTON_LONG:

            if (!pressed)
            {
                buttonState = BUTTON_RELEASE_DEBOUNCE;
                buttonStateStartTime = now;
            }
            else if ((now - lastRepeatTime) >= REPEAT_INTERVAL)
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

            if ((now - pressStartTime) >= STUCK_TIME)
            {
                buttonState = BUTTON_STUCK;

                showMessage("BUTTON", "STUCK");

                Serial.println("BUTTON STUCK");
            }

            break;


        case BUTTON_RELEASE_DEBOUNCE:

            if (pressed)
            {
                if (wasLongPress)
                    buttonState = BUTTON_LONG;
                else
                    buttonState = BUTTON_PRESSED;
            }
            else if ((now - buttonStateStartTime) >= DEBOUNCE_TIME)
            {
                if (wasLongPress)
                {
                    showMessage("LONG PRESS", "Released");

                    Serial.println("LONG PRESS Released");
                }
                else
                {
                    showMessage("SHORT PRESS", "Detected");

                    Serial.println("SHORT PRESS Detected");
                }

                buttonState = BUTTON_RELEASED;
                repeatCount = 0;
            }

            break;


        case BUTTON_STUCK:

            if (!pressed)
            {
                buttonState = BUTTON_RELEASE_DEBOUNCE;
                buttonStateStartTime = now;
            }

            break;
    }
}

/* TASK 3 MEASUREMENT */
void printMeasurement()
{
    unsigned long now = millis();

    if ((now - lastMeasurementPrint) < 500UL)
        return;

    lastMeasurementPrint = now;

    unsigned long widthCopy;
    unsigned long periodCopy;

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

        Serial.print(" us  Period: ");
        Serial.print(periodCopy);

        Serial.print(" us  Frequency: ");
        Serial.print(frequency);

        Serial.println(" Hz");
    }
}

/* SETUP */
void setup()
{
    lcd.begin(16, 2);

    Serial.begin(9600);

    /* GPIO safe startup */
    digitalWrite(GPIO_PIN, LOW);
    pinMode(GPIO_PIN, OUTPUT);

    gpioState = GPIO_DISABLED;

    /* TASK 3 */
    pinMode(INPUT_PIN, INPUT);

    pinMode(TEST_SIGNAL_PIN, OUTPUT);
    digitalWrite(TEST_SIGNAL_PIN, LOW);

    attachInterrupt(
        digitalPinToInterrupt(INPUT_PIN),
        measureSignal,
        CHANGE
    );

    lastSignalToggle = micros();

    showMessage("MODULE 4", "FIRMWARE READY");

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

/* LOOP */
void loop()
{
    updateTestSignal();

    updateButton();

    printMeasurement();
}

