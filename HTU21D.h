#ifndef _HTU21D_H_
#define _HTU21D_H_
//set i2c address
#define   HTU21D_I2C_ADDR 0x40

//get temperature without holding
#define   HTU21D_TEMP     0xF3
//get humidity without holding
#define   HTU21D_HUMID    0xF5

#define   TCA9548A_ADDR    0x70

// Get temperature
double getTemperature(int fd);

// Get humidity
double getHumidity(int fd);

double calculateDewPoint(double T, double RH);

#endif
