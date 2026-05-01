#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include <timer.h>
#include <adc.h>
#include <pwm.h>
#include <dac.h>
#include <i2c.h>

#include <util/delay.h>

#include <RFM69.h>
#include <RFM69_ATC.h>
#include <SPIFlash.h>

typedef enum button_debounce_state {
  wait_press, 
  debounce_press, 
  wait_release, 
  debounce_release
} DebounceState;

typedef enum RX_TX_state {
  rx,
  tx,
} CommState;

volatile uint8_t sample_buffer[10];
volatile int buffer_front = 0;
volatile int buffer_back = 0;
volatile uint8_t last_sample;

volatile DebounceState curr_button_state = wait_press;
volatile CommState rx_tx_state = rx;
volatile bool button_pressed = 0;

volatile bool sample_ready = 0;

int main() {

  initTimer1();
  initTimer0();
  initPWMTimer3();
  initADC();
  initI2C();

  Serial.begin(9600);

  sei();

  //Main Loop
  while(1) {

    if (sample_ready) {
        sample_ready = 0;
        write_to_DAC(last_sample);
        sample_ready = 0;
    }

    //Debounce state machine and RX_TX logic
    if(curr_button_state == debounce_press) {
      delayUs(50);
      rx_tx_state = tx;
      curr_button_state = wait_release;
    }
    else if(curr_button_state == debounce_release) {
      delayUs(50);
      rx_tx_state = rx;
      curr_button_state = wait_press;
    }

  }

  return 0;

}

//ADC ISR triggered when sample ready
ISR(ADC_vect) {

  last_sample = ADCH; //Load value from ADC
  sample_ready = 1; //Sample flag
}

//ISR for ADC timer, just used to reset flag
ISR(TIMER1_COMPB_vect) {
    //Empty ISR to reset flag
}

//Switch ISR
ISR(INT0_vect) {
  if(curr_button_state == wait_press) {
    curr_button_state = debounce_press;
  }
  else if(curr_button_state == wait_release) {
    curr_button_state = debounce_release;
  }
}