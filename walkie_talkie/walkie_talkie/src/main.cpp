#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

#include <timer.h>
#include <adc.h>
#include <pwm.h>
#include <dac.h>
#include <i2c.h>
#include <usart.h>

#include <util/delay.h>

#define BUFFER_SIZE 32

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

volatile uint8_t sample_buffer[BUFFER_SIZE];
volatile uint8_t last_sample;
volatile int head_index;
volatile int tail_index;
volatile bool buffer_empty_flag = 0;
volatile bool write_to_dac_flag = 0;

volatile DebounceState curr_button_state = wait_press;
volatile CommState rx_tx_state = rx;

volatile bool sample_ready = 0;
volatile uint8_t recieve_data = 0;
volatile bool recieved_flag = 0;

//RF24 radio(7, 8); // CE, CSN pins

uint8_t pop_from_buffer() {
  uint8_t data = sample_buffer[head_index];
  if(head_index == tail_index) {
    if(buffer_empty_flag) {
      return 128; //Silence
    }

    buffer_empty_flag = 1;
    return data;
  }
  head_index = (head_index + 1) % BUFFER_SIZE;
  buffer_empty_flag = 0;

  return data;
}

int append_to_buffer(uint8_t data) {
  int new_tail = (tail_index + 1) % BUFFER_SIZE;

  if(new_tail == head_index) {
    return 1; //Overflow
  }
  
  sample_buffer[new_tail] = data;
  tail_index = new_tail;
  return 0;
}

int main() {

  initTimer1();
  initTimer0();
  initADC();
  initI2C();
  init_USART();

  uint8_t data_from_buffer = 0;

  //radio.begin();
  //radio.setPALevel(RF24_PA_LOW);
  //radio.setDataRate(RF24_1MBPS);

  Serial.begin(9600);

  sei();

  //Main Loop
  while(1) {

    if(sample_ready) {
      if(rx_tx_state == tx) {
        usart_send_byte(last_sample);
      }
      sample_ready = 0;
    }

    if(write_to_dac_flag) {
      if(rx_tx_state == rx) {
        data_from_buffer = pop_from_buffer();
        write_to_DAC(data_from_buffer);
      }
      else {
        write_to_DAC(128); //Silence
      }
      write_to_dac_flag = 0;
    }

    if(recieved_flag) {
      if(rx_tx_state == rx) {
        append_to_buffer(last_sample);
      }
      recieved_flag = 0;
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
  write_to_dac_flag = 1;
}

//Switch ISR
ISR(PCINT1_vect) {
  if(curr_button_state == wait_press) {
    curr_button_state = debounce_press;
  }
  else if(curr_button_state == wait_release) {
    curr_button_state = debounce_release;
  }
}

ISR(USART1_RX_vect) {
    recieve_data = UDR1; //Get data
    recieved_flag = 1; //Set flag to poll in main
}