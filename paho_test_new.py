#Subscriber script for Prototype 2
#for Remote environmental monitiring system for Data centre
#By Nicholas Clinch
#15/02/21
import paho.mqtt.client as MQTT #for MQTT stuff
import RPi.GPIO as GPIO
import time #for sleep
import datetime #for time for data logs

#globals
#set pin for relay
channel = 21
#setup GPIOs
GPIO.setmode(GPIO.BCM)
GPIO.setup(channel, GPIO.OUT)
GPIO.output(channel, GPIO.HIGH)
#set location of USB
fb = open('/media/pi/GAMES/pi-stuff/DataLog.txt', 'a')
#set temperature and humidity limits
TEMP_LIM = 24
HUM_LIM = 55
#once connected to GPIO server
#activate GPIO
def pin_on(pin):
        GPIO.output(pin,GPIO.LOW)
#deactivate GPIO
def pin_off(pin):
        GPIO.output(pin,GPIO.HIGH)
#When connected to broker
def on_connect(self, client, userdata, rc):
    print ("Connected")
	print ("Subscribing to SHT21/Temperature")
	#subscribe to SHT21/Temperature
	self.subscribe("SHT21/Temperature")
	print ("Subscribing to SHT21/Humidity")
	#subscribe to SHT21/Humidity
	self.subscribe("SHT21/Humidity")
	#write to USB that new connection has been made
	fb.write('*******\nNew connection made\n*******\n')


#when message is recieved from broker
def on_message(client, userdata, msg):
	#print recieved message
	print (msg.topic + " " + msg.payload)
	#take note of time and date
	x =str(datetime.datetime.now())
	#write payload to USB
	fb.write(msg.topic + '\t' + msg.payload + '\t\t')
	fb.write(x)
	fb.write('\n')

	#if topic is from SHT/Temperature
	if msg.topic == 'SHT21/Temperature':
		Temp = float(msg.payload)
		#if payload is greater than limit
		if Temp > TEMP_LIM:
			#turn on GPIO
			pin_on(channel)
		else:
			#tunr off GPIO
			pin_off(channel)
	elif msg.topic == 'SHT21/Humidity':
                Hum = float(msg.payload)
                if Hum > HUM_LIM:
                        pin_on(channel)
                else:
                        pin_off(channel)

#once dsconnected, stop loop
def on_disconnect(client, userdata, rc):
	client.loop_stop()

#main script
#settup client functions
client = MQTT.Client()
client.username_pw_set(username="pi",password="raspberry")
client.on_connect = on_connect
client.on_message = on_message
client.on_disconnect = on_disconnect
#declare broker being subscribed too
#client.connect("test.mosquitto.org" , 1883, 60)
client.connect("localhost" , 1883, 60)
client.loop_start()

#always run
while True:
	if __name__ == '__main__':
        	try:
			#rest for 0.1 second, may not be required
                        time.sleep(0.1)
		#if interupt is triggered, reset GPIOs and print error message
                except KeyboardInterrupt:
			client.disconnect()
                        GPIO.cleanup()
			print ()
                        print ("Called Keyboard Interupt, Aborting Program")
                        exit()
