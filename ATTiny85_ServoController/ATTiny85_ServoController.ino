/*
 * ATTiny85_DCC_ServoController.ino
 * 
 * Copyright 2017 Otto Schreibke <oschreibke@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * The license can be found at https://www.gnu.org/licenses/gpl-3.0.txt
 * 
 * 
 */

#include <SimpleServo.h>

#define Servo_Pin 0
#define Interrupt_Pin 2
#define LedPin 1

// adjust as needed
#define SERVO_ON_ANGLE 45
#define SERVO_OFF_ANGLE 135
#define SERVO_DELAY 25

volatile boolean servoState = true;
boolean lastServoState = true;

struct {
    int targetAngle;
    int currentAngle;
    int nextServoMove;
    SimpleServo  servo;
} servoInfo;

unsigned long lastMillis = millis();
int nextHeartbeat = 0;
bool ledPinStatus = false;

// Interrupt Service Routine (ISR)
ISR(PCINT0_vect) {
    // handle pin change interrupt here
    servoState = (PINB >> Interrupt_Pin) & 0x01; //PINB is the register to read the state of the pins
}

void setup() {

    // set up the heartbeat
    nextHeartbeat = 500;
    ledPinStatus = false;
    pinMode(LedPin, OUTPUT); // LED
    digitalWrite(LedPin, ledPinStatus);

    // set up the servo
    servoInfo.servo.attach(Servo_Pin);
    servoInfo.targetAngle =  servoInfo.currentAngle = SERVO_ON_ANGLE;
    servoInfo.servo.write(servoInfo.targetAngle);
    servoInfo.nextServoMove = SERVO_DELAY;

    // enable interrupt for pin...
    pinMode(Interrupt_Pin, INPUT);
    GIMSK = 0b00100000;    // turns on pin change interrupts
    PCMSK = 0b00000100;    // turn on interrupts on pin 2
    sei();                 // enables interrupts

}

void loop() {

    unsigned long millisNow = millis();
    bool milliTick = (lastMillis != millisNow);
    lastMillis = millisNow;

    if (servoState != lastServoState) {
        lastServoState = servoState;
        servoInfo.targetAngle = (servoState) ? SERVO_ON_ANGLE : SERVO_OFF_ANGLE;
    }

    // gentle servo move
    if (servoInfo.targetAngle != servoInfo.currentAngle) {
        if (milliTick) {
            if (!--servoInfo.nextServoMove) {
                servoInfo.nextServoMove = SERVO_DELAY;
                if (servoInfo.currentAngle < servoInfo.targetAngle)
                    servoInfo.currentAngle++;
                else
                    servoInfo.currentAngle--;
                servoInfo.servo.write(servoInfo.currentAngle);
            }
        }
    }

    // heartbeat: blink the led on pin1
    if (milliTick) {
        if (!--nextHeartbeat) {
            nextHeartbeat =  500;
            ledPinStatus = !ledPinStatus;
            digitalWrite(LedPin, ledPinStatus);
        }
    }

}
