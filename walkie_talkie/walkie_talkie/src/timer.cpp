#include "Timer.h"

/*
Timer 1 is setting the sample rate (currently 4kHz) of the ADC. Auto-Triggering set to Timer1 Output Compare
*/

void initTimer1(){
	//set Timer1 to CTC mode
    TCCR1A &= ~(1<<WGM10); 
    TCCR1A &= ~(1<<WGM11);  
    TCCR1B |= (1<<WGM12);

    //Prescalar = 8
    TCCR1B &= ~(1<<CS10);
    TCCR1B |= (1<<CS11);
    TCCR1B &= ~(1<<CS12);

    //Value of 499
    //Top byte of output compare 
    OCR1AH = 0xF3;

    //Bottom byte of output compare
    OCR1AL= 0x01;
}