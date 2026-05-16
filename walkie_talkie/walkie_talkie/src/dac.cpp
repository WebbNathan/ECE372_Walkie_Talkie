#include <dac.h>
#include <i2c.h>

#include <Arduino.h>

#define MCP4725_ADDR 0x60

void write_to_DAC(uint8_t sample) {
    
    uint16_t value12 = ((uint16_t)sample) << 4; //Shift 8 bit sample up to 12 bit

    Start_I2C_Trans(MCP4725_ADDR); //Start transmission by sending the DACS address
    write(value12 >> 8); //Write Top 4 bits
    write(value12 & 0xFF); //Write Bottom 4
    StopI2C_Trans(); //Stop I2C transmission

}