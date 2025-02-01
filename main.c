#include <wiringPi.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdint.h>

#define DS 0 // data pin
#define STCP 3 // latch pin
#define SHCP 2 // clock pin
#define LED 5
#define BUTTON 6

#define LSBFIRST 0
#define MSBFIRST 1

volatile int led_state = 0;
volatile int running = 1;
volatile int button_pressed = 0;

// this is for wrong type of 7 segment display oopsie
const unsigned char segments[] = {
    0b0110000,
    0b1101110,
    0b1111001,
    0b0110011,
    0b1011011,
    0b1110000,
};

void buttonISR() {
    button_pressed = 1;
}

void handle_signal(int signal) {
    running = 0;
}

void shiftOut(int data_pin, int clock, int bit, unsigned char data) {
    for (int i = 0; i < 8; i++) {
        if (bit == LSBFIRST) {
            digitalWrite(data_pin, (data >> i) & 0x01);
        } else {
            digitalWrite(data_pin, (data >> (7 - i)) & 0x01);
        }

        digitalWrite(clock, HIGH);
        delayMicroseconds(10);
        digitalWrite(clock, LOW);
        delayMicroseconds(10);
    }
}

int main() {
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed\n");
        return 1;
    }

    pinMode(DS, OUTPUT);
    pinMode(SHCP, OUTPUT);
    pinMode(STCP, OUTPUT);

    digitalWrite(STCP, LOW);

    // this should display a 0 for a CA 7 segment display
    unsigned char testPattern = 0b11000000; 
    digitalWrite(STCP, LOW);
    shiftOut(DS, SHCP, 1, testPattern);
    digitalWrite(STCP, HIGH);

    printf("Shifted out test pattern: 0x%02X\n", testPattern);
    sleep(5);

    return 0;
}