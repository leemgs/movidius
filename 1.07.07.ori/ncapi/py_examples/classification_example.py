from mvnc import mvncapi as mvnc
import sys
import numpy
import cv2
import time
import csv
import os
import sys

if len(sys.argv) != 2:
	print ("Usage: enter 1 for Googlenet, 2 for Alexnet, 3 for Squeezenet")
	sys.exit()
if sys.argv[1]=='1':
	network="googlenet"
elif sys.argv[1]=='2':
	network='alexnet'
elif sys.argv[1]=='3':
	network='squeezenet'
else:
	print ("Usage: enter 1 for Googlenet, 2 for Alexnet, 3 for Squeezenet")
	sys.exit()

# get labels
labels_file='../tools/synset_words.txt'
labels=numpy.loadtxt(labels_file,str,delimiter='\t')
# configuration NCS
mvnc.SetGlobalOption(mvnc.GlobalOption.LOGLEVEL, 2)
devices = mvnc.EnumerateDevices()
if len(devices) == 0:
	print('No devices found')
	quit()
device = mvnc.Device(devices[0])
device.OpenDevice()
opt = device.GetDeviceOption(mvnc.DeviceOption.OPTIMISATIONLIST)

if network == "squeezenet":
	network_blob='../networks/SqueezeNet/graph'
	dim=(227,227)
elif network=="googlenet":
	network_blob='../networks/GoogLeNet/graph'
	dim=(224,224)
elif network=='alexnet':
	network_blob='../networks/AlexNet/graph'
	dim=(227,227)
#Load blob
with open(network_blob, mode='rb') as f:
	blob = f.read()
graph = device.AllocateGraph(blob)
graph.SetGraphOption(mvnc.GraphOption.ITERATIONS, 1)
iterations = graph.GetGraphOption(mvnc.GraphOption.ITERATIONS)

ilsvrc_mean = numpy.load('../mean/ilsvrc12/ilsvrc_2012_mean.npy').mean(1).mean(1) #loading the mean file
img = cv2.imread('../images/cat.jpg')
img=cv2.resize(img,dim)
img = img.astype(numpy.float32)
img[:,:,0] = (img[:,:,0] - ilsvrc_mean[0])
img[:,:,1] = (img[:,:,1] - ilsvrc_mean[1])
img[:,:,2] = (img[:,:,2] - ilsvrc_mean[2])
graph.LoadTensor(img.astype(numpy.float16), 'user object')
output, userobj = graph.GetResult()
order = output.argsort()[::-1][:6]
print('\n------- predictions --------')
for i in range(1,6):
	print ('prediction ' + str(i) + ' is ' + labels[order[i]])
graph.DeallocateGraph()
device.CloseDevice()
