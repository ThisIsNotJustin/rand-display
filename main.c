#include <wiringPi.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdint.h>

#define DS 0 // data pin
#define STCP 3 // latch pin
#define SHCP 2 // clock pin
#define BUTTON 6

#define LSBFIRST 0
#define MSBFIRST 1

volatile int led_state = 0;
volatile int running = 1;
volatile int button_pressed = 0;

// CA 7 segment display
const unsigned char segments[] = {
    // 0b11000000, // 0
    0b11111001, // 1
    0b10100100, // 2
    0b10110000, // 3
    0b10011001, // 4
    0b10010010, // 5
    0b10000010, // 6
    // 0b11111000, // 7
    // 0b11000000, // 8
    // 0b10011000, // 9
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

int display_zero() {
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

    sleep(5);

    return 0;
}

/*
    Has an error where a dice press gets skipped
    interrupt may be counted as a debounce delay
    resulting in the occasional bouble roll from 1 press?
*/
int main() {
    if(wiringPiSetup() == -1) {
        printf("wiringpi setup failed\n");
        return 1;
    }

    pinMode(LED, OUTPUT);
    pinMode(BUTTON, INPUT);
    pullUpDnControl(BUTTON, PUD_UP);
    
    pinMode(DS, OUTPUT);
    pinMode(SHCP, OUTPUT);
    pinMode(STCP, OUTPUT);

    digitalWrite(STCP, LOW);
    shiftOut(DS, SHCP, MSBFIRST, 0x00);
    digitalWrite(STCP, HIGH);

    if (wiringPiISR(BUTTON, INT_EDGE_FALLING, &buttonISR) < 0) {
        printf("Unable to setup ISR\n");
        return 1;
    }

    srand(time(NULL));

    printf("Roll Dice\n");

    signal(SIGINT, handle_signal);

    while (running){
        if(button_pressed) {
            button_pressed = 0;

            delay(50);

            int diceval = (rand() % 6);
            unsigned char dice = segments[diceval];

            digitalWrite(STCP, LOW);
            shiftOut(DS, SHCP, MSBFIRST, dice);
            digitalWrite(STCP, HIGH);
            delayMicroseconds(10);
            digitalWrite(STCP, LOW);

            printf("Dice should be: %d\n", diceval + 1);
            delay(300);
        }

        delay(10);
    }

    printf("Stopped Program\n");

    return 0;
}