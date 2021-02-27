#include <unistd.h>

#include "wiringPi.h"
#include "wiringPiI2C.h"

#include "HTU21D.h"

// Get temperature
double getTemperature(int fd)
{
	unsigned char buf [3];
	//write to slave to get temperature
	wiringPiI2CWrite(fd, HTU21D_TEMP);
	//wait for measurement
	delay(100);
	//read from slave
	read(fd, buf, 2);
	//convert 2 8 bit bytes into 1 16 bit number,
	//and remove last two bits (only 14 bit resolution)
	unsigned int temp = (buf [0] << 8 | buf [1]) & 0xFFFC;
	// Convert sensor reading into temperature.
	// See page 10 of the datasheet
	double tSensorTemp = temp / 65536.0;
	//return temprature in degrees C
	return -46.85 + (175.72 * tSensorTemp);
}

// Get humidity
double getHumidity(int fd)
{
	unsigned char buf [3];
	//write to slave to get relative humidity
	wiringPiI2CWrite(fd, HTU21D_HUMID);
	//wait for measurement
	delay(100);
	//read from sensor
	read(fd, buf, 2);
	//convert 2 8 bit bytes into 1 16 bit number,
	//and remove last four bits (only 12 bit resolution)
  	unsigned int humid = (buf [0] << 8 | buf [1]) & 0xFFF0;
	// Convert sensor reading into humidity.
	// See page 10 of the datasheet
	double tSensorHumid = humid / 65536.0;
	return -6.0 + (125.0 * tSensorHumid);
}
