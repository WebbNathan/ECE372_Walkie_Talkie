#ifndef USART_H
#define USART_H

#include <avr/io.h>

void initUSART();
void usart_send_byte(uint8_t data);

#endif