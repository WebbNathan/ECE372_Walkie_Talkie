#include <led.h>

//PIN 22
void initLEDPA0() {
    DDRA |= (1 << DDA0); //Set led to output
}

void led_on(){
    PORTA |= (1 << PORTA0); //Set LED logic to one (turns on LED)
}
void led_off(){
    PORTA &=~(1 << PORTA0); //Set LED logic to zero
}