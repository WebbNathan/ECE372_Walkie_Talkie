#include "switch.h"
#include <avr/io.h>

void initSwitchPJ0(){
    
    //Enables data direction to input and turns pull up resistor
    DDRJ &= ~(1<<DDJ0);
    PORTJ |= (1<<PORTJ0);

    //Enable PCINT9 interupt
    PCMSK1 |= (1 << PCINT9);
    PCICR |= (1 << PCIE1);
}