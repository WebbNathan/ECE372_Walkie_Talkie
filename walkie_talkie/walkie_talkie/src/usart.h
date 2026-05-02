#ifndef USART_H
#define USART_H

#include <avr/io.h>

void init_USART();
void usart_send_byte(uint8_t data);

#endif