#include <avr/io.h>
#include <avr/interrupt.h>

#include <timer.h>
#include <adc.h>
#include <dac.h>
#include <i2c.h>
#include <usart.h>
#include <switch.h>
#include <led.h>

#define BUFFER_SIZE 64 //Sets max samples in buffer
#define START_LEVEL 10 //Sets the min amount of samples in buffer to begin writing to DAC
#define BUFFER_MASK (BUFFER_SIZE - 1) //Used for calulating the amount of samples in buffer

typedef enum button_debounce_state { //For switch debouncing
  wait_press, 
  debounce_press, 
  wait_release, 
  debounce_release
} DebounceState;

typedef enum RX_TX_state { //Comm is half duplex with push-to-talk transmission
  rx,
  tx,
} CommState;

volatile uint8_t sample_buffer[BUFFER_SIZE];
volatile uint8_t last_sample;
volatile uint8_t head_index;
volatile uint8_t tail_index;

volatile DebounceState curr_button_state = wait_press;
volatile DebounceState curr_mute_button_state = wait_press;
volatile CommState rx_tx_state = rx;

volatile bool write_to_dac_flag = 0;
volatile bool sample_ready = 0;
volatile bool ready_to_playback = 0;
volatile bool muted_flag = 0;

uint8_t pop_from_buffer() { //Pop from ring buffer
  if(head_index == tail_index) { //This implies buffer is empty
      return 128; //Silence
  }

  uint8_t data = sample_buffer[head_index];
  
  head_index = (head_index + 1) % BUFFER_SIZE; //Mod BUFFER_SIZE as this is a ring buffer and must be able to loop
  return data;
}

int append_to_buffer(uint8_t data) {
  int new_tail = (tail_index + 1) % BUFFER_SIZE; //Ring buffer so mod BUFFER_SIZE

  if(new_tail == head_index) { //Implies an overflow so dont append anything
    return 1; //Overflow
  }
  
  sample_buffer[tail_index] = data; //Append data
  tail_index = new_tail; //Set new tail
  return 0;
}

uint8_t get_buffer_size() {
  return (tail_index - head_index) & BUFFER_MASK; //Mask effectivley loops the diffrence mod BUFFER_SIZE
}

int main() {

  //Module intilization
  initTimer1();
  initTimer0();
  initADC();
  initI2C();
  initUSART();
  initSwitchPJ0();
  initSwitchPK0();
  initLEDPA0();

  led_off(); //Initialize LED to off

  sei();

  //Main Loop
  while(1) {

    uint8_t buffer_size = get_buffer_size();
    
    if(buffer_size > START_LEVEL) { //Ensuring a min of START_LEVEL samples in buffer before writing
      ready_to_playback = 1;
    }

    if(buffer_size == 0) { //If no samples left then stop writing and continue refilling
      ready_to_playback = 0;
    }

    if(sample_ready) { //Sample ready set in ADC ISR
      if(rx_tx_state == tx) { //Make sure we are in transmitting state
        usart_send_byte(last_sample); //Send sample to radio module
      }
      sample_ready = 0; //Reset flag
    }

    if(write_to_dac_flag) { //Check if we are ready to write to DAC
      if (rx_tx_state == rx) { //Make sure we are recieving
        if (!ready_to_playback) { //If buffer is not full enough just write silence
          write_to_DAC(128); // silence while filling
        }
        else {
          uint8_t data = pop_from_buffer(); //Pop a sample from buffer
          if(!muted_flag) { //Set by mute button
            write_to_DAC(data); //Write data if not muteed
          }
          else {
            write_to_DAC(128); //Write silence if muted
          }
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
      if (!(PINJ & (1 << PINJ0))) { //
        rx_tx_state = tx;
        led_on(); //Turn on transmit LED
        curr_button_state = wait_release;
      }
      else {
        curr_button_state = wait_press;
      }
    }
    else if(curr_button_state == debounce_release) {
      delayUs(10000); //Debounce delay
      if((PINJ & (1 << PINJ0))) { //Making sure the button was lifted
        rx_tx_state = rx;
        led_off(); //Turn off transmit LED
        curr_button_state = wait_press;
      }
      else {
        curr_button_state = wait_release;
      }
    }

    //Debounce state machine for mute button
    if(curr_mute_button_state == debounce_press) {
      delayUs(10000);
      curr_mute_button_state = wait_release;
    }
    else if(curr_mute_button_state == debounce_release) {
      delayUs(10000);
      muted_flag = !muted_flag;
      curr_mute_button_state = wait_press;
    }

  }

  return 0;
}

//ADC ISR triggered when sample ready
ISR(ADC_vect) {
  last_sample = ADCH; //Load value from ADC
  sample_ready = 1; //Sample flag
}

//ISR for ADC timer, just used to reset flag and set a flag that indicates ready to write to DAC
ISR(TIMER1_COMPB_vect) {
  write_to_dac_flag = 1;
}

//Push-to-talk ISR
ISR(PCINT1_vect) {
  //Debouncing logic
  if(curr_button_state == wait_press) {
    curr_button_state = debounce_press;
  }
  else if(curr_button_state == wait_release) {
    curr_button_state = debounce_release;
  }
}

//Muted button ISR
ISR(PCINT2_vect) {
  //Debounding logic
  if(curr_mute_button_state == wait_press) {
    curr_mute_button_state = debounce_press;
  }
  else if(curr_mute_button_state == wait_release) {
    curr_mute_button_state = debounce_release;
  }
}

ISR(USART1_RX_vect) {
    uint8_t recieve_data = UDR1; //Get data
    if(rx_tx_state == rx) { //Only write signals if we are receiving
        append_to_buffer(recieve_data);
    }
}