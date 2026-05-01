#include <i2c.h>
#include <Arduino.h>


void initI2C(){
    //Turns on I2C
    PRR0 &= ~(1 << PRTWI);

    TWSR = 0x00;   // prescaler = 1
    TWBR = 12;     // ~400 kHz for 16 MHz CPU

    //Enable I2C
    TWCR = (1 << TWINT) | (1 << TWEN);
}

//The address for the imu is 0x1D
void Start_I2C_Trans(unsigned char SLA){

    uint8_t addressByte = (SLA << 1) & 0xFE;

    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA); // reset flag and intiate a start condition


    while (!(TWCR & (1 << TWINT)));  // wait while flag is low


    TWDR = (SLA << 1) & (0xFE); //load i2c address and write bit (0)
    TWCR = (1 << TWINT) | (1 << TWEN);  // clear flag and enable TWI


    while (!(TWCR & (1 << TWINT)));  // wait while flag is low
}

void StopI2C_Trans(){
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO); //send stop condition

    while (TWCR & (1 << TWSTO));  // wait until stop if finished
}

uint8_t write(unsigned char data){
    //Load the data register with data
    TWDR = data; 

    TWCR = (1 << TWINT) | (1 << TWEN);  // clear flag and enable TWI

    while(!(TWCR & (1 << TWINT)));  // wait while flag is low

    return TWSR & 0xF8;
}