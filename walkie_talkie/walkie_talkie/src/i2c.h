#include <avr/io.h>

void initI2C();
void Start_I2C_Trans(unsigned char SLA);
void StopI2C_Trans();
uint8_t write(unsigned char data);
void i2c_write_to(unsigned char SLA, unsigned char address, unsigned char data);
void Read_from(unsigned char SLA, unsigned char MEMADDRESS);
unsigned char Read_data();