import numpy as np
import math
import pylsl
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from PIL import Image
import cv2
import csv

file = open("Kinect Images/Skeleton.csv", "w", newline='')
writer = csv.writer(file)
bodyparts = ["pelvis", "naval", "chest", "neck", "lclavicle", "lshoulder",
             "lelbow", "lwrist", "lhand", "lhandtip", "lthumb", "rclavicle",
             "rshoulder", "relbow", "rwrist", "rhand", "rhandtip", "rthumb",
             "lhip", "lknee", "lankle", "lfoot", "rhip", "rknee", "rankle",
             "rfoot", "head", "nose", "leye", "lear", "reye", "rear"]
datapoints = ["px", "py", "pz", "rw", "rx", "ry", "rz", "conf"]
firstrow = ["Order", "TStamp"]
for bodypart in bodyparts:
    for datapoint in datapoints:
        firstrow.append(bodypart + datapoint)
writer.writerow(firstrow)
print("Waiting for streams")
streamC = pylsl.resolve_byprop("type", "Color Image")
streamS = pylsl.resolve_byprop("type", "Skeleton Data")
inletC = pylsl.StreamInlet(streamC[0])
inletS = pylsl.StreamInlet(streamS[0])
print("Streams opened")
sample, ts = inletS.pull_sample()
sample, ts = inletC.pull_sample()
print("first pulls")

skeletonPointReached = False
colorPointReached = False

while not skeletonPointReached:
    sample, ts = inletS.pull_sample()
    print(ts)
    if sample[7] > 5:
        skeletonPointReached = True
print("Skeleton Synchronized")

while not colorPointReached:
    sample, ts = inletC.pull_sample()
    print(ts)
    if sample[4] > 1:
        colorPointReached = True
print("Color Synchronized")

i = 0
print(i)
sampleC, tsC = inletC.pull_sample()
print(tsC)
sampleS, tsS = inletS.pull_sample()
print(tsC, tsS)
while sampleS != None and sampleC != None:
    #width = 1280
    #width = 1920
    #width = 2560
    width = 2048
    arr = np.array(sampleC).astype(np.uint8)
    arr = arr.reshape(int(len(sampleC)/(width*4)), width*4)
    b = arr[:, ::4]
    g = arr[:, 1::4]
    r = arr[:, 2::4]
    a = arr[:, 3::4]
    arrays = [r, g, b, a]
    image = np.stack(arrays, axis=2)
    color = Image.fromarray(image, mode='RGBA')
    color.save("Kinect Images\color" + str(tsC) + ".png", "PNG")
    for joint in range(32):
        image[max(0, int(sampleS[joint*8 + 1]) - 10):min(image.shape[0], int(sampleS[joint*8+1]) + 10),
              max(0, int(sampleS[joint*8]) - 10):min(image.shape[1], int(sampleS[joint*8]) + 10)] = [0, 255, 0, 0]
    colorTracked = Image.fromarray(image, mode = 'RGBA')
    colorTracked.save("Kinect Images/track" + str(tsS) + ".png", "PNG")
    myList = [str(i)]
    myList.append(str(tsS))
    for elt in sampleS:
        myList.append(str(elt))
    writer.writerow(myList)
    i = i+1
    print(i)
    sampleC, tsC = inletC.pull_sample(timeout=10)
    print(tsC)
    sampleS, tsS = inletS.pull_sample(timeout=10)
    print(tsC, tsS)
