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
fb = open('/media/pi/NICKPIUSB/pi-stuff/DataLog.txt', 'a')
#set temperature and humidity limits
TEMP_MIN_LIM = 18
TEMP_MAX_LIM = 27
HUM_LIM = 60
DEW_MAX_LIM = 15
DEW_MIN_LIM = -9
#last recorded temperature/humdity
#override flag
override_flag = 0
Temp_flag = 0
Dew_flag = 0
Hum_flag = 0

#typical values for aberdeen
Outdoor_Dew = 12.00
#once connected to GPIO server
#activate GPIO
def pin_on(pin):
        GPIO.output(pin,GPIO.LOW)
#deactivate GPIO
def pin_off(pin):
        GPIO.output(pin,GPIO.HIGH)
#When connected to broker
def on_connect(client, userdata, flags, rc):
    print ("Connected")
        
    #subscribe to all 1st level subtopics of 'SHT21'
    print ("Subscribing to SHT21/#")
    client.subscribe("home/SHT21/#")
        
    #subscribe to all 1st level subtopics of 'bridge'
    print ("Subscribing to bridge/+")
    client.subscribe("bridge/+")
        
    #write to USB that new connection has been made
    fb.write('*******\nNew connection made\n*******\n')


#when message is recieved from broker
def on_message(client, userdata, msg):
    global override_flag
    global Temp_flag
    global Hum_flag
    global Dew_flag
    #print recieved message
    print (msg.topic + " " + msg.payload)
    #take note of time and date
    x =str(datetime.datetime.now())
    #write payload to USB
    fb.write(msg.topic + '\t' + msg.payload + '\t\t')
    fb.write(x)
    fb.write('\n')

    #if topic is from SHT/Temperature
    if msg.topic == 'home/SHT21/Temperature' and override_flag == 0:
        Temp = float(msg.payload)
        #if payload is greater than limit
        if Temp > TEMP_MAX_LIM:
            #turn on GPIO
            print ("WARNING: Temperature above recommended levels")
            pin_on(channel)
                
        elif Temp < TEMP_MIN_LIM and Hum_flag ==0 and Dew_flag ==0:
            print ("WARNING: Temperature below recommended levels")
            #turn off GPIO
            pin_off(channel)
        else:
            Temp_flag = 0
                
    elif msg.topic == 'home/SHT21/DewPoint' and override_flag == 0:
        Dew = float(msg.payload)
        #if payload is greater than limit
        global Outdoor_Dew
        if Dew>Outdoor_Dew:
            if Dew > DEW_MAX_LIM:
                print ("WARNING: Dew Point above recommended levels")
                #turn on GPIO
                pin_on(channel)
                dew_flag = 1
            elif Dew < DEW_MIN_LIM and Hum_flag ==0 and Temp_flag ==0:
                print ("WARNING: Dew Point below recommended levels")
                #turn off GPIO
                pin_off(channel)
            else:
                dew_flag = 0
        else:
            if Dew > DEW_MAX_LIM and Hum_flag ==0 and Temp_flag ==0:
                print ("WARNING: Dew Point above recommended levels")
                #turn on GPIO
                pin_off(channel)
                dew_flag = 1
            elif Dew < DEW_MIN_LIM :
                print ("WARNING: Dew Point below recommended levels")
                #turn off GPIO
                pin_on(channel)
            else:
                dew_flag = 0

    elif msg.topic == 'home/SHT21/Humidity'and override_flag == 0:
        Hum = float(msg.payload)
        
        if Hum > HUM_LIM:
            print ("WARNING: Relative Humidity above recommended levels")
            pin_on(channel)
            Hum_flag = 1
        else:
            Hum_lim = 0

    elif msg.topic == 'bridge/override':
        override_flag = 1 
        if msg.payload == "ON":
            print ("Manual Override: Turning on Actuator")
            pin_on(channel)
            
        elif msg.payload == "OFF":
            pin_off(channel)
            print ("Manual Override: Turning off Actuator")

    elif msg.topic == 'bridge/auto':
        if msg.payload == "ON":
            override_flag = 0
            print ("Resuming Automatic Control")
        
    elif msg.topic == 'home/SHT21/Dewpoint/Outside':
        Outdoor_Dew = msg.payload

#once dsconnected, stop loop
def on_disconnect(client, userdata, rc):
    client.loop_stop()

#main script
#settup client functions
client = MQTT.Client()
#client.username_pw_set(username="pi",password="raspberry")
client.on_connect = on_connect
client.on_message = on_message
client.on_disconnect = on_disconnect
#declare broker being subscribed too
client.connect("localhost" , 1883, 60)
#client.connect("localhost" , 1883, 60)
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
