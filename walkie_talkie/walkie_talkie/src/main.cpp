#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include <timer.h>
#include <adc.h>

volatile uint8_t in_sample_buffer[80]; //80/4000 ~ 20ms of audio
volatile unsigned int in_sample_buffer_count = 0;

int main() {

  initTimer1();
  initADC();

  sei();

  //Main Loop
  while(1) {

    if(in_sample_buffer_count >= 80) {
      //Send Packet
      in_sample_buffer_count = 0;
    }

  }

  return 0;

}

ISR(ADC_vect) {

  //Only take bottom 8 bits (8 bit audio)
  in_sample_buffer[in_sample_buffer_count] = ADCL;
  in_sample_buffer_count += 1;

}

