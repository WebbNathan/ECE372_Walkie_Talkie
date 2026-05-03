#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include <timer.h>
#include <adc.h>
#include <dac.h>
#include <i2c.h>
#include <usart.h>
#include <switch.h>

#define BUFFER_SIZE 64
#define START_LEVEL 10
#define BUFFER_MASK (BUFFER_SIZE - 1) 

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
volatile uint8_t head_index;
volatile uint8_t tail_index;
volatile bool buffer_empty_flag = 0;
volatile bool write_to_dac_flag = 0;

volatile DebounceState curr_button_state = wait_press;
volatile CommState rx_tx_state = rx;

volatile bool sample_ready = 0;
volatile bool ready_to_playback = 0;

uint8_t pop_from_buffer() {
  if(head_index == tail_index) {
      return 128; //Silence
  }

  uint8_t data = sample_buffer[head_index];
  
  head_index = (head_index + 1) % BUFFER_SIZE;
  return data;
}

int append_to_buffer(uint8_t data) {
  int new_tail = (tail_index + 1) % BUFFER_SIZE;

  if(new_tail == head_index) {
    return 1; //Overflow
  }
  
  sample_buffer[tail_index] = data;
  tail_index = new_tail;
  return 0;
}

uint8_t get_buffer_size() {
  return (tail_index - head_index) & BUFFER_MASK;
}

int main() {

  initTimer1();
  initTimer0();
  initADC();
  initI2C();
  init_USART();
  initSwitchPJ0();

  uint8_t data_from_buffer = 0;

  sei();

  //Main Loop
  while(1) {

    uint8_t buffer_size = get_buffer_size();
    
    if(buffer_size > START_LEVEL) {
      ready_to_playback = 1;
    }

    if(buffer_size == 0) {
      ready_to_playback = 0;
    }

    if(sample_ready) {
      if(rx_tx_state == tx) {
        usart_send_byte(last_sample);
      }
      sample_ready = 0;
    }

    if(write_to_dac_flag) {
      if (rx_tx_state == rx) {
        if (!ready_to_playback) {
          write_to_DAC(128); // silence while filling
        }
        else {
          uint8_t data = pop_from_buffer();
          write_to_DAC(data);
        }

      }
      else {
        ready_to_playback = 0;
        write_to_DAC(128);
      }
      write_to_dac_flag = 0;
    }

    //Debounce state machine and RX_TX logic
    if(curr_button_state == debounce_press) {
      delayUs(10000);
      if (!(PINJ & (1 << PINJ0))) {
        rx_tx_state = tx;
        curr_button_state = wait_release;
      }
      else {
        curr_button_state = wait_press;
      }
    }
    else if(curr_button_state == debounce_release) {
      delayUs(10000);

      if((PINJ & (1 << PINJ0))) {
        rx_tx_state = rx;
        curr_button_state = wait_press;
      }
      else {
        curr_button_state = wait_release;
      }
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
    uint8_t recieve_data = UDR1; //Get data
    if(rx_tx_state == rx) {
        append_to_buffer(recieve_data);
    }
}