#! /usr/bin/env python
# -*- coding: utf-8 -*-

import sys, serial, time, struct
#from PyQt4.QtCore import pyqtSlot
from PyQt4 import QtGui, QtCore
import numpy as np
import matplotlib.pyplot as plt

serial_port = "/dev/pts/2"
#serial_port = "/dev/ttyACM3"
#serial_port = "/dev/rfcomm4"
baudrate = 9600

class MissionControl(QtGui.QWidget):
	def __init__(self):
		super(MissionControl, self).__init__()
		self.initUI()
        
	def initUI(self):
		self.start = True
		self.todo = ""
		self.reference = "";
		# Initialize serial connection
		self.connection = serial.Serial(serial_port, baudrate, timeout=1)
		self.connection.flushOutput()
		self.connection.flushInput()

		# Set window properties
		self.setGeometry(0, 0, 1150, 500)

		# Forward Button
		btnUp = QtGui.QPushButton('Forward', self)
		btnUp.move(100, 0)  
		btnUp.pressed.connect(self.forward)
		btnUp.released.connect(self.stop)

		# Reverse Button
		btnReverse = QtGui.QPushButton('Reverse', self)
		btnReverse.move(100, 100)  
		btnReverse.pressed.connect(self.back)
		btnReverse.released.connect(self.stop)

		# Left Button
		btnLeft = QtGui.QPushButton('Left', self)
		btnLeft.move(0, 50)  
		btnLeft.pressed.connect(self.left)
		btnLeft.released.connect(self.stop)

		# Right Button
		btnRight = QtGui.QPushButton('Right', self)
		btnRight.move(200, 50)  
		btnRight.pressed.connect(self.right)
		btnRight.released.connect(self.stop)

		# Stop Button
		btnStop = QtGui.QPushButton('Stop', self)
		btnStop.move(100, 50)  
		btnStop.clicked.connect(self.stop)

		# Distance
		btnDistance = QtGui.QPushButton('Distance', self)
		btnDistance.move(50, 150)  
		btnDistance.clicked.connect(self.distance_start)

		# Spectrum
		btnDistance = QtGui.QPushButton('Spectrum', self)
		btnDistance.move(150, 150)  
		btnDistance.clicked.connect(self.spectrum_start)
		
		# Spectrum
		btnDistance = QtGui.QPushButton('Reference', self)
		btnDistance.move(150, 200)  
		btnDistance.clicked.connect(self.spectrum_reference)
		
		# Spectrum
		btnDistance = QtGui.QPushButton('LEDs on', self)
		btnDistance.move(50, 300)  
		btnDistance.clicked.connect(self.leds_on)
		
		# Spectrum
		btnDistance = QtGui.QPushButton('LEDs off', self)
		btnDistance.move(150, 300)  
		btnDistance.clicked.connect(self.leds_off)
		
		# Exposure Slider
		self.exposure = QtGui.QSlider(self)
		self.exposure.move(0,200)
		self.exposure.setMinimum(1)
		self.exposure.setMaximum(255)
		self.exposure.valueChanged.connect(self.change_exposure)
		
		# Exposure Label (milliseconds)
		self.expLabel = QtGui.QLabel(self)
		self.expLabel.move(50,200)
		self.expLabel.resize(100,50)
		self.expLabel.setText(str(self.exposure.value()*100)+' ms')

		self.graph = QtGui.QLabel(self)
		self.graph.move(350,0)
		
		x = range(0,128,20)
		y = range(0,128,20)
		
		self.distancePlot(x,y)
		self.plotImage()
		self.show()
		
	# move the robot forward
	@QtCore.pyqtSlot()
	def change_exposure(self):
		self.expLabel.setText(str(self.exposure.value()*100)+' ms')

	# move the robot forward
	@QtCore.pyqtSlot()
	def forward(self):
		self.connection.write('U')
		print("U")
	
	# stop the robot
	@QtCore.pyqtSlot()
	def stop(self):
		self.connection.write('C')
		print("S")
	
	# turn the robot to the left
	@QtCore.pyqtSlot()
	def left(self):
		self.connection.write('L')
		print("L")
	
	# turn the robot to the right
	@QtCore.pyqtSlot()
	def right(self):
		self.connection.write('R')
		print("R")
	
	# move the robot backwards
	@QtCore.pyqtSlot()
	def back(self):
		self.connection.write('D')
		print("D")
	
	# move the robot backwards
	@QtCore.pyqtSlot()
	def leds_on(self):
		self.connection.write('F')
		print("D")
		
	# move the robot backwards
	@QtCore.pyqtSlot()
	def leds_off(self):
		self.connection.write('G')
		print("D")	
	
	# send distance measurement command
	@QtCore.pyqtSlot()
	def distance_start(self):
		self.connection.write('A')
		print("A")
		self.current_timer = QtCore.QTimer()
		self.current_timer.timeout.connect(self.distance_read)
		self.current_timer.setSingleShot(True)
		self.current_timer.start(1000)

	# read distance measurement
	@QtCore.pyqtSlot()
	def distance_read(self):
		print("read callback")
		x = range(0,128)
		y = self.connection.readline().split(";")
		print(y);
		del y[-1]
		print(len(x))
		print(len(y))
		y = map(int, y)
		self.distancePlot(x,y)
		self.plotImage()
	
	def spectrum_reference(self):
		self.todo = "reference";
		self.spectrum_start()
	
	# start exposure
	@QtCore.pyqtSlot()
	def spectrum_start(self):
		self.connection.write('E')
		print("E")
		self.connection.write(chr(self.exposure.value()))
		print(chr(self.exposure.value()))
		self.current_timer = QtCore.QTimer()
		self.current_timer.timeout.connect(self.spectrum_read)
		self.current_timer.setSingleShot(True)
		self.current_timer.start(self.exposure.value()*100+500)
		
	# read spectrum
	@QtCore.pyqtSlot()
	def spectrum_read(self):
		y = self.connection.readline().split(";")
		del y[-1]
		print(y)
		y = map(int, y)
		print(len(y))
		x = range(0,256)
		print(len(x))
		if(self.todo=="reference"):
			self.reference = y
			print(self.reference)
		else:
			print(self.reference)
			for i in range(0,256):
				y[i] = float(y[i]) / float(self.reference[i])
			print(y)
		self.spectrumPlot(x,y)
		self.plotImage()
	
	def plotImage(self):
		print("datei anzeigen")
		self.pixmap = QtGui.QPixmap('tmp.png')
		self.graph.setPixmap(self.pixmap)
		self.show()
		print("datei angezeigt")
		
	def distancePlot(self,x,y):
		my_dpi = 96	
		fig = plt.figure(figsize=(800/my_dpi, 500/my_dpi), dpi=my_dpi)
		plt.axis((0,128,0,1024))
		plt.scatter(x,y)
		fig.suptitle('Distance Map', fontsize=20)
		plt.xlabel('Angle [rad]', fontsize=18)
		plt.ylabel('Distance [cm]', fontsize=16)
		fig.savefig('tmp.png')
		print("datei gespeichert")
		
	def spectrumPlot(self,x,y):
		my_dpi = 96	
		fig = plt.figure(figsize=(800/my_dpi, 500/my_dpi), dpi=my_dpi)
		if(self.todo == "reference"):
			plt.axis((0,256,0,1024))
			self.todo = ""
		else:
			plt.axis((0,256,0,1))
		plt.scatter(x,y)
		fig.suptitle('Spectrum', fontsize=20)
		plt.xlabel('Pixel', fontsize=18)
		plt.ylabel('Intensity', fontsize=16)
		fig.savefig('tmp.png')
		print("datei gespeichert")
     
def main():
	app = QtGui.QApplication(sys.argv)
	ex = MissionControl()
	sys.exit(app.exec_())


if __name__ == '__main__':
	main()    
