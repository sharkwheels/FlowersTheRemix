### IMPORTS ###############################################
from __future__ import print_function

import myo as libmyo; libmyo.init()
import logging
import inspect
import math 
import os
import serial
import struct
import sys
import time
import threading
import random
from random import randint

### Logging ###################################################

logging.basicConfig(level=logging.DEBUG, format='[%(levelname)s] (%(threadName)-10s) %(message)s')

### GLOBALS ######################################################

isActiveOne = False
isActiveTwo = False

### ok, so these also have to be global because they reset every time my function runs #

closedFlowers = ['flower1','flower2']		
openFlowers = []	
demoEnd = False										

### PYTHON SERIAL OUT #########################################

ser = serial.Serial('/dev/cu.usbmodem14211', 9600)
time.sleep(3)

if ser:
	print("Connection to /dev/cu.usbmodem14231 established succesfully!\n")
else:
	print("Could not make connect to /dev/cu.usbmodem1421")

### MYO DATA HELPERS ############################################

def parse_data(playerData):
	"""
	Player Data is sent through as a list of Dicts. This function will parse desired data out of the structure
	and send commands down to the Euler function, then to the pass to function. 
	"""
	#print(playerData)
	#print(" ")

	player1 = None
	player2 = None

	### dirty AF but it will work for now...
	try:
		player1 = playerData[0]
	except IndexError:
		pass
	try:
		player2 = playerData[1]
	except IndexError:
		pass

	### Orientation and Pose for Player 1

	if player1:

		#print(player1['orientation'])

		p1quatX = player1['orientation'][0]
		p1quatY = player1['orientation'][1]
		p1quatZ = player1['orientation'][2]
		p1quatW =  player1['orientation'][3]

		p1Euler = euler_angles(p1quatX, p1quatY ,p1quatZ ,p1quatW)

		p1roll = p1Euler[0]
		p1pitch = p1Euler[1]
		p1yaw = p1Euler[2]

		p1pose = player1['pose']
		
	### Orientation and Pose for player2 

	if player2:

		#print(player2['orientation'])

		p2quatX = player2['orientation'][0]
		p2quatY = player2['orientation'][1]
		p2quatZ = player2['orientation'][2]
		p2quatW =  player2['orientation'][3]

		p2Euler = euler_angles(p2quatX,p2quatY,p2quatZ,p2quatW)

		p2roll = p2Euler[0]
		p2pitch = p2Euler[1]
		p2yaw = p2Euler[2]

		p2pose = player2['pose']

	"""
	print("*************************")
	print('P1-roll_w: ', p1roll)
	print('P1-pitch_w: ', p1pitch)
	print('P1-yaw_w: ', p1yaw)
	print('p1-pose:', p1pose)
	print("*************************")
	print('P2-roll_w: ', p2roll)
	print('P2-pitch_w: ', p2pitch)
	print('P2-yaw_w: ', p2yaw)
	print('p2-pose:', p2pose)
	"""
	if player1 and player2:
		#print("demoEnd: {0}".format(demoEnd))
		if not demoEnd:

			## open flower one ########################
			## both arms are pointed down and hands are a fist

			if p2pitch == 7 and p2pose == 'fist' and p1pitch == 7 and p1pose == 'fist' and not isActiveOne:
				demo_logic("f1open")
				
			
			## open flower two #################
			## both arms are held up over head no pose
			
			if p2pitch == 10 and p1pitch == 10 and not isActiveTwo:
				demo_logic("f2open")


			## open both flowers #################
			## both arms are held out to the side and hands are a fist.

			if p2pitch == 8 and p2pose == 'fist' and p1pitch == 8 and p1pose == 'fist' and not isActiveOne and not isActiveTwo:
				demo_logic("openBoth")
			
			
	else:
		print("I'm sorry, you need two players for this to work")
	
	

def euler_angles(quatX,quatY,quatZ,quatW):
	"""
	Calculates Euler Angles from myo quaternion, and returns a list of roll, pitch, yaw
	"""

	roll1 = 2.0 * (quatW * quatX + quatY * quatZ)
	roll2 = (1.0 - 2.0) * (quatX * quatX + quatY * quatY)

	yaw1 = 2.0 * (quatW * quatZ + quatX * quatY)
	yaw2 = 1.0 - 2.0 * (quatY * quatY + quatZ * quatZ)

	roll = math.atan2(roll1,roll2)
	pitch = math.asin(max(-1.0, min(1.0, 2.0 *(quatW * quatY - quatZ * quatX))))
	yaw = math.atan2(yaw1,yaw2)

	roll_w = int(((roll + (math.pi)) / (math.pi * 2.0) * 18))
	pitch_w = int(pitch + (math.pi/2.0)/math.pi * 18)
	yaw_w = int(yaw + (math.pi / (math.pi * 2.0)) * 18)

	eulerAngles = [roll_w,pitch_w,yaw_w]
	return eulerAngles

#### GAME HELPER FUNCTIONS #################################################################

def open_flower(targetF,timerCom,serialCom):
	try:
		idxCF = closedFlowers.index(targetF) #(target)
		print(idxCF)
		toOpen = closedFlowers[idxCF]
		print(toOpen)
		openFlowers.append(toOpen)
		closedFlowers.remove(toOpen)

		ser.write(struct.pack('>B', serialCom))

		## call the timer to start
		start(targetF,timerCom)
		

	except IndexError or ValueError(e):  ## expect just error(e)
		pass
	print("open: {0} | closed: {1}".format(openFlowers,closedFlowers))

def close_flower(targetF,serialCom):
	try:
		idxOF = openFlowers.index(targetF) #(target)
		print(idxOF)
		toClose = openFlowers[idxOF]
		print(toClose)
		closedFlowers.append(toClose)
		openFlowers.remove(toClose)
		ser.write(struct.pack('>B', serialCom))
	except IndexError or ValueError(e):  ## expect just error(e)
		pass
	print("close: {0} | closed: {1}".format(openFlowers,closedFlowers))


#### MAIN GAME LOGIC #################################################################

def demo_logic(command): 
	"""
	How each Flower in the game behaves with the timer and other flowers. 
	"""
	global closedFlowers
	global openFlowers
	global demoEnd
	

	#if command:
		#print("GLOBAL: open: {0} | closed: {1}".format(openFlowers,closedFlowers))


	### FLOWER 1 LOGIC ###########################################################################
	
	if command == 'f1open':
		#print(command)
		if 'flower1' in closedFlowers:
			open_flower('flower1',1,11)
	
	elif command == 'f1close':
		## if timer is running
		if not isActiveOne:
			close_flower('flower1',12)


	### FLOWER 2 LOGIC #########################################################################################

	if command == 'f2open':
		if 'flower2' in closedFlowers: 
			open_flower('flower2',2,21)
	elif command == 'f2close':
		if not isActiveTwo:
			close_flower('flower2',22)


	### OPEN BOTH #########################################################################################

	if command == "openBoth":
		## if all(flower1 and flower2) are both closed
		if all(y in closedFlowers for y in ['flower1', 'flower2']):
			ser.write(struct.pack('>B', 70))
			demoEnd = True;

	
	### END THE DEMO #########################################################################################

	if(demoEnd):
		# sort the list on each pass to keep flower1 / 2 etc...in order.
		print("demoEnd_start: {0}".format(demoEnd))
		closedFlowers.sort()

		print("closedFlowers: {0}".format(closedFlowers))
		print("openFlowers: {0}".format(openFlowers))
		demoEnd = False;

		## make sure that was done
		print("demoEnd_end: {0}".format(demoEnd))
		time.sleep(5) # sleep for 5 seconds
	
	
### TIMER STUFF ######################################################

def countdown(pName,com):

	global isActiveOne
	global isActiveTwo

	if com == 1:
		isActiveOne = True
	elif com == 2:
		isActiveTwo = True

	print("{0} countdown - command{1} ".format(pName,com))
	retry = 0
	while True:
		toRun = randint(10,20) # random time to be open for
		print("{0}:{1}:{2}".format(pName,toRun,retry))
		retry += 1
		if retry > toRun:
			if com == 1:
				isActiveOne = False
				demo_logic("f1close")
			elif com == 2:
				isActiveTwo = False
				demo_logic("f2close")
			break
		time.sleep(1)
	print("{0} timer-done".format(pName))
	print(isActiveOne,isActiveTwo)

def start(pName,com):
	print("starting countdown for: ",pName)
	t = threading.Thread(target=countdown,args=(pName,com))
	t.setName(pName)
	t.setDaemon(True)
	t.start()

### RUNTIME ###############################################

def main():
	"""
	This is the main feed run loop. It initiates the myo hub and sets things up to assign the right data to each myo.
	Still to do: Clean this arse up, add either a start movement or a start "button" or pose. 
	"""

	### get the myo feed ###
	try:
		hub = libmyo.Hub()
	except MemoryError:
		print("Myo Hub could not be created. Make sure Myo Connect is running.")
		return
		
	feed = libmyo.device_listener.Feed()
	hub.run(1000, feed)

	### connect to myos (no god damn mac addresses. ugh.) ###

	## if 
	try:
		print("Waiting to connect...")
		myo = feed.wait_for_single_device(5) ## pass the seconds to wait
		if not myo:
			print("No myo connected after 5 seconds")
			return

		if not ser: 
			print("There is no serial connection.")
			return

		# send a shuffle command
		ser.write(struct.pack('>B', 70))

		rawMyos = feed.get_devices() 
		namedMyos = []

		### basically this a fudge, if only one myo is connected it will be player1
		### if two myos are connected they will be 1 and 2 respectivelly
		### but they might flip flop each time the program is re-started
		### PROGRESS. I'll have to look at that C++ library hack for this. Which is nut bar. 

		for idx, val in enumerate(rawMyos,start=1):
			print(idx, val)

			## if the myo is connected add the idx and myo to a list
			if val.connected:
				namedMyos.append([idx,val])	
			
			## buzz them diff depending on which player they are
			if idx == 1 and val.connected:
				val.vibrate('long')
				print("player1 -long-")
				
			if idx == 2 and val.connected:
				val.vibrate('short')
				val.vibrate('short')
				print("player2 -shortx2-")

		while hub.running and myo.connected:
			playerData = []
			### dirty but it will work for now.
			try:
				p1 = namedMyos[0][1]
				p1.set_stream_emg(libmyo.StreamEmg.enabled)
				player1 = {'name': 'player1','pose': p1.pose, 'orientation':p1.orientation, 'acceleration':p1.acceleration, 'arm':p1.arm, 'gyro':p1.gyroscope, 'emg': p1.emg,'xdir':p1.x_direction}
				playerData.append(player1)
			except IndexError:
				pass

			try:
				p2 = namedMyos[1][1]
				p2.set_stream_emg(libmyo.StreamEmg.enabled)
				player2 = {'name': 'player2','pose': p2.pose, 'orientation':p2.orientation,'acceleration':p2.acceleration, 'arm':p2.arm, 'gyro':p2.gyroscope, 'emg': p2.emg, 'xdir': p2.x_direction}
				#print(player2)
				playerData.append(player2)
			except IndexError:
				pass	
			parse_data(playerData)
			time.sleep(0.1) ## this is just here to slow it down while debugging
			
		print("Disconnecting")

	except KeyboardInterrupt:
		print("KeyboardInterrupt")
	finally:
		print("Shutting Down Hub")
		hub.shutdown()

if __name__ == "__main__":
	main()
