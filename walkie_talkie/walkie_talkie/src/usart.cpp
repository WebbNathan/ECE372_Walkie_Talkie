#include <usart.h>

void init_USART() {
    UCSR1A |= (1 << U2X1);   // double speed mode
    UBRR1H = 0;
    UBRR1L = 16;             // 115200 baud at 16 MHz with U2X = 1  

    //Enable transmit and recieve
    UCSR1B = (1 << RXEN1) | (1 << TXEN1);

    //8 data bits, no parity, 1 stop bit
    UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);

    UCSR1B |= (1 << RXCIE1);   //Enable receive interupt
}

void usart_send_byte(uint8_t data) {
    while (!(UCSR1A & (1 << UDRE1))); //Wait for buffer to empty

    UDR1 = data;
}