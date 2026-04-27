#include "switch.h"
#include <avr/io.h>

void initSwitchPD0(){
    
    //Enables data direction to input and turns pull up resistor
    DDRD &= ~(1<<DDD0);
    PORTD |= (1<<PORTD0);

    //Set interupt to trigger on any logical change
    EICRA |= (1 << ISC00);
    EICRA &= ~(1 << ISC01);

    //Enable INT0 interupt
    EIMSK |= (1 << INT0);
}