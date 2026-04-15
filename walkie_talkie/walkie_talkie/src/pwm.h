#ifndef PWM_H
#define PWM_H

#include <avr/io.h>

void initPWMTimer3(); 
void set_duty_cycle_from_sample(uint8_t sample);

#endif