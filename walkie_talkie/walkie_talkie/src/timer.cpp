#include "timer.h"

/*
Timer 1 is setting the sample rate (currently 4kHz) of the ADC. Auto-Triggering set to Timer1 Output Compare
*/

void initTimer1() {
	//set Timer1 to CTC mode
    TCCR1A &= ~(1<<WGM10); 
    TCCR1A &= ~(1<<WGM11);  
    TCCR1B |= (1<<WGM12);
    TCCR1B &= ~(1 << WGM13);

    //Prescalar = 8
    TCCR1B &= ~(1<<CS10);
    TCCR1B |= (1<<CS11);
    TCCR1B &= ~(1<<CS12);

    TCNT1 = 0;

    TIMSK1 |= (1 << OCIE1B);

    OCR1A = 499;
    OCR1B = 259;
}

void initTimer0() {
    //set Timer0 to CTC mode
    TCCR0A &= ~(1<<WGM00); 
    TCCR0A |= (1<<WGM01);  
    TCCR0B &= ~(1<<WGM02); 

    //Prescalar = 8
    TCCR0B &= ~(1<<CS00);
    TCCR0B |= (1<<CS01);
    TCCR0B &= ~(1<<CS02);

    //starting count 
    TCNT0 = 0;

    //ending count 2
    OCR0A = 1;


}

void delayUs(unsigned int delay) {
    unsigned int dCnt = 0;

    //starting count 0
    TCNT0 = 0;

    //set compare flag
    TIFR0 |= (1<<OCF0A);

    while(dCnt < delay) {
        if (TIFR0 & (1 << OCF0A)) {
            //1ms elapsed
            dCnt++;
            //clear flag
            TIFR0 |= (1 << OCF0A);
        }
    }
}