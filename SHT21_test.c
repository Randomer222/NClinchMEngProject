/*
SHT21 sensor  with MQTT communication
By Nicholas Clinch, expanded from 
code by Leon Avavi
16/02/21
*/

//header files
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mosquitto.h>  //for MQTT communication
#include "wiringPi.h"   // for pi commands
#include "wiringPiI2C.h" //for I2C commands


#include "HTU21D.h"  //header with Temperature/humidity conversion formulas



int main ()
{
	//setup I2C communication
	wiringPiSetup();
	int fd = wiringPiI2CSetup(HTU21D_I2C_ADDR);

	//if I2C cannot be accessed
	if ( 0 > fd )
	{
		fprintf (stderr, "Unable to open I2C device: %s\n", strerror (errno));
		exit (-1);
	}

	//get temperature and humidity
	float Temperature =getTemperature(fd);
	float Humidity = getHumidity(fd);

	//print to screen
	printf("%5.2f C\n", Temperature);
	printf("%5.2f %%rh\n", Humidity);

	//convert float to string (may not be nessisary, as should always be between 4 and 7 bytes)
        int length_t = snprintf( NULL, 0, "%5.2f", Temperature );
        char* str_t = malloc( length_t + 1 );
        snprintf( str_t, length_t + 1, "%5.2f", Temperature );

	//convert float to string
        int length_h = snprintf( NULL, 0, "%5.2f", Humidity );
        char* str_h = malloc( length_h + 1 );
        snprintf( str_h, length_h + 1, "%5.2f", Humidity );

	// mosquitto publish setup
        int rc;
        struct mosquitto *mosq;


	//initialise mosquitto library
        mosquitto_lib_init();
        mosq = mosquitto_new("Publisher-test", true, NULL);

	//setup connection to broker at this IP address, port 1883, with 60 second timeout
        rc = mosquitto_connect(mosq, "192.168.1.19", 1883, 60);

	//if connection is not successful, print error message and abort program
        if (rc != 0)
        {
                printf("Error, could not connect to broker, Error code : %d\n", rc);
                mosquitto_destroy(mosq);
                return -1;
        }

	//print success message
        printf("Connected to Broker\n");

	//publish temperature under topic SHT21/Temperature
        mosquitto_publish(mosq, NULL, "SHT21/Temperature",length_t, str_t, 0, false);
	//publish Humidity under topic SHT21/Humidity
	mosquitto_publish(mosq, NULL, "SHT21/Humidity", length_h, str_h, 0, false);

	//disconnect from broker and destroy struct
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);

	//cleanup and free mallocs
        mosquitto_lib_cleanup();
        free(str_t);
	free(str_h);

	return 0;
}


