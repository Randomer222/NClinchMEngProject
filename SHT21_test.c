/*
SHT21 sensor  with MQTT communication
By Nicholas Clinch, expanded from
code by Leon Avavi
16/02/21
modified on 23/02/21
*/
//if you make changes to the file, must run this command in SHT21 dir:
//gcc -c -o HTU21D.o HTU21D.c -lm && gcc -c -o SHT21_test.o SHT21_test.c -lm && gcc -o SHT21_test HTU21D.o SHT21_test.o -lwiringPi -lmosquitto -lm

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

#define DELAY 60000

void TCA9548A(u_int8_t bus)
{
	printf("\nSensor %d\n", bus);
	int fd = wiringPiI2CSetup(TCA9548A_ADDR);
	wiringPiI2CWrite(fd, 1 << bus);
}

void on_connect(struct mosquitto* mosq, void* obj, int reason_code)
{
	/* Print out the connection result. mosquitto_connack_string() produces an
	 * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	 * clients is mosquitto_reason_string().
	 */
	printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
	if (reason_code != 0) {
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */
		mosquitto_disconnect(mosq);
	}
	//print success message
	printf("Connected to Broker\n");

	/* You may wish to set a flag here to indicate to your application that the
	 * client is now connected. */
}

void on_disconnect(struct mosquitto* mosq, void* obj, int reason_code)
{
	printf("Disconnected from Broker");
	//disconnect from broker and destroy struct
	mosquitto_destroy(mosq);
	//cleanup and free mallocs
	mosquitto_lib_cleanup();
	exit(0);
}

int main()
{
	//setup I2C communication
	wiringPiSetup();
	//initialise mosquitto library
	mosquitto_lib_init();
	int fd = wiringPiI2CSetup(HTU21D_I2C_ADDR);

	// mosquitto  setup
	int rc;
	struct mosquitto* mosq;
	float Temperature;
	float Humidity;
	float DewPoint;

	mosq = mosquitto_new("Publisher", true, NULL);
	if (mosq == NULL) {
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_disconnect_callback_set(mosq, on_disconnect);
	//int SSL = mosquitto_tls_set(mosq, "/home/pi/SHT21/mosquitto.crt", NULL, NULL, NULL, NULL);
	//printf("%d\n",SSL);
	//setup connection to broker at this IP address, port 1883, with 60 second timeout
	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	//if connection is not successful, print error message and abort program
	if (rc != 0)
	{
		printf("Error, could not connect to broker, Error code : %d\n", rc);
		mosquitto_destroy(mosq);
		return -1;
	}
	//if I2C cannot be accessed
	if (0 > fd)
	{
		fprintf(stderr, "Unable to open I2C device: %s\n", strerror(errno));
		exit(-1);
	}
	int loop = mosquitto_loop_start(mosq);
		if (loop != MOSQ_ERR_SUCCESS)
		{
			mosquitto_destroy(mosq);
			fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
			return 1;
		}

	while (1)
	{
		//get temperature and humidity
		TCA9548A(2);
		Temperature = getTemperature(fd);
		Humidity = getHumidity(fd);
		DewPoint = calculateDewPoint(Temperature, Humidity);

		//print to screen
		printf("%5.2f C\n", DewPoint);
		int length_d_1 = snprintf( NULL, 0, "%5.2f", DewPoint );
        char* str_d1 = malloc( length_d + 1 );
        snprintf( str_d, length_d + 1, "%5.2f", DewPoint );
        mosquitto_publish(mosq, NULL, "home/SHT21/Dewpoint/Outside", length_d_1, str_d1, 0, false);
        free(str_d1);
		
		TCA9548A(1);
		Temperature = getTemperature(fd);
		Humidity = getHumidity(fd);
		DewPoint = calculateDewPoint(Temperature, Humidity);

		//print to screen
		printf("%5.2f C\n", Temperature);
		printf("%5.2f %%rh\n", Humidity);
		printf("%5.2f C\n", DewPoint);

		//convert float to string (may not be nessisary, as should always be between 4 and 7 bytes)
		int length_t = snprintf(NULL, 0, "%5.2f", Temperature);
		char* str_t = malloc(length_t + 1);
		snprintf(str_t, length_t + 1, "%5.2f", Temperature);

		//convert float to string
		int length_h = snprintf(NULL, 0, "%5.2f", Humidity);
		char* str_h = malloc(length_h + 1);
		snprintf(str_h, length_h + 1, "%5.2f", Humidity);
		
		int length_d = snprintf( NULL, 0, "%5.2f", DewPoint );
        char* str_d = malloc( length_d + 1 );
        snprintf( str_d, length_d + 1, "%5.2f", DewPoint );

		//publish temperature under topic SHT21/Temperature
		mosquitto_publish(mosq, NULL, "home/SHT21/Temperature", length_t, str_t, 0, false);
		//publish Humidity under topic SHT21/Humidity
		mosquitto_publish(mosq, NULL, "home/SHT21/Humidity", length_h, str_h, 0, false);
		//publish Dew point under topic SHT21/DewPoint
		mosquitto_publish(mosq, NULL, "home/SHT21/DewPoint", length_d, str_d, 0, false);
		
		free(str_d);
		free(str_t);
		free(str_h);
		delay(DELAY);
	}
}
