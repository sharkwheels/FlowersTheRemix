import serial
import struct
import time

### PYTHON SERIAL OUT #########################################

ser = serial.Serial('/dev/cu.usbmodem1421', 9600)
time.sleep(3)

if ser:
	print("Connection to /dev/cu.usbmodem14211 established succesfully!\n")
else:
	print("Could not make connect to /dev/cu.usbmodem14211")
while 1:
	s = int(input("> "))
	arf = struct.pack('>B', s)
	ser.write(arf)
	print(arf)