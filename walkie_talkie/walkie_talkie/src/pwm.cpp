#include <pwm.h>

void initPWMTimer3() {
    DDRE |= (1 << DDE5); //Set pin 3 to output

    //Setting Fast PWM (8 bit) 0xFF (255) Top. Should create ~62.5kHz PWM Freq
    TCCR3A |= (1 << WGM30);
    TCCR3A &= ~(1 << WGM31);
    TCCR3B |= (1 << WGM32);
    TCCR3B &= ~(1 << WGM33);

    //Set timer 3 prescaler to 1
    TCCR3B |= (1 << CS30);
    TCCR3B &= ~((1 << CS31) | (1 << CS32));

    //Initilize duty cycle to 50%
    //50% of 255 ~ 128
    OCR3C = 128;
}

void set_duty_cycle_from_sample(uint8_t sample) {
    OCR3C = (uint16_t)(sample);
}