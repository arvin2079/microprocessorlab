#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

char cycles[] PROGMEM = {0x09, 0x05, 0x06, 0x0A};

// 3 secsors activation would be indicated by 3 bit. bit 0 for right sensor, 1 bit for middle one and 2 bit for left one.
// two wheels status would be indicated bu 2 bit which bit 0 is for right wheel and bit 1 for left wheel.
// also somehow we need to perform random move, random move would be indicated by 00.
// 001 -> 10
// 010 -> 11
// 011 -> 10
// 100 -> 01
// 101 -> 00 or random move
// 110 -> 01
// 111 -> 00 or random move
char sensors_wheels_stat[] PROGMEM = {10, 11, 10, 01, 00, 01, 00}; 

int main(void)
{
    while (1)
    {
        
    }
}