#include <ADC.h>

void initADC() {
    ADMUX |= (1 << REFS0); //Set reference voltage to AVcc keeps MUX = 00000 (A0)
    ADMUX &= ~(1 << REFS1);

    DIDR0 |= (1 << ADC7D); //Set pin A0 to analog input

    ADMUX &= ~(1 << ADLAR); //Right adjust result

    ADCSRB &= ~(1 << ADTS1);
    ADCSRB |= ((1 << ADTS0) | (1 << ADTS2)); //Trigger sampling on timer one compare match B

    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); //Set prescaler to 128

    ADCSRA |= (1 << ADIE); //Enable interupt on ADC

    ADCSRA |= (1 << ADEN) | (1 << ADATE); //Enable ADC
    ADCSRA |= (1 << ADSC); //Start conversion
    
}