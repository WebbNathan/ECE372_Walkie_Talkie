#include "switch.h"
#include <avr/io.h>

//PIN 15
void initSwitchPJ0(){ //Transmission push button
    
    //Enables data direction to input and turns pull up resistor
    DDRJ &= ~(1<<DDJ0);
    PORTJ |= (1<<PORTJ0);

    //Enable PCINT9 interupt
    PCMSK1 |= (1 << PCINT9);
    PCICR |= (1 << PCIE1);
}

//PIN A8
void initSwitchPK0(){ //Mute button
    //Enables data direction to input and turns pull up resistor
    DDRK &= ~(1<<DDK0);
    PORTK |= (1<<PORTK0);

    //Enable PCINT9 interupt
    PCMSK2 |= (1 << PCINT16);
    PCICR |= (1 << PCIE2);
}