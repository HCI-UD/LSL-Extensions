import numpy as np
import math
import pylsl
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from PIL import Image
import cv2

def sampleToString(sample):
    minY=0
    maxY=0
    minX=0
    maxX=0
    minZ=0
    maxZ=0
    result = ""
    names = ["pelvis", "navel", "chest", "neck", "left clavicle",
             "left shoulder", "left elbow", "left wrist", "left hand",
             "left handtip", "left thumb", "right clavicle", "right shoulder",
             "right elbow", "right wrist", "right hand", "right handtip",
             "right thumb", "left hip", "left knee", "left ankle", "left foot",
             "right hip", "right knee", "right ankle", "right foot", "head",
             "nose", "left eye", "left ear", "right eye", "right ear"]
    for i in range(len(names)):
        result += names[i] + ":\n\tp-x: " + str(sample[i*8])
        result += "\n\tp-y: " + str(sample[i*8 + 1])
        result += "\n\tp-z: " + str(sample[i*8 + 2])
        result += "\n\to-w: " + str(sample[i*8 + 3])
        result += "\n\to-x: " + str(sample[i*8 + 4])
        result += "\n\to-y: " + str(sample[i*8 + 5])
        result += "\n\to-z: " + str(sample[i*8 + 6])
        result += "\n\tconfidence: "
        if int(sample[i*8 + 7]) == 0:
            result += "none\n"
        elif int(sample[i*8 + 7]) == 1:
            result += "low\n"
        elif int(sample[i*8 + 7]) == 2:
            result += "medium\n"
        elif int(sample[i*8 + 7]) == 3:
            result += "high\n"
        else:
            result += "unknown\n"
        if (sample[i*8] < minX):
            minX = sample[i*8]
        elif sample[i*8] > maxX:
            maxX = sample[i*8]
        if (sample[i*8 + 1] < minY):
            minY = sample[i*8 + 1]
        elif sample[i*8 + 1] > maxY:
            maxY = sample[i*8 + 1]
        if (sample[i*8 + 2] < minZ):
            minZ = sample[i*8 + 2]
        elif sample[i*8 + 2] > maxZ:
            maxZ = sample[i*8 + 2]
    print("Min X:", minX, "Max X:", maxX)
    print("Min Y:", minY, "Max Y:", maxY)
    print("Min Z:", minZ, "Max Z:", maxZ)
    print()
    return result
        
print("Waiting for Skeleton Stream")
streams = pylsl.resolve_byprop("type", "Skeleton Data")
inlet = pylsl.StreamInlet(streams[0])
print("Opened Skeleton Stream")
sample, ts = inlet.pull_sample()
while sample != None:
    resultsString = "timestamp = " + str(ts) +"\n"
    resultsString += sampleToString(sample)
    f = open("Saved Images\data" + str(ts) + ".txt", "w")
    f.write(resultsString)
    f.close()
    sample, ts = inlet.pull_sample(timeout=10)
print("Done with Skeleton")
