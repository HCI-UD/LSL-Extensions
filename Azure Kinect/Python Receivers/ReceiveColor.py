import numpy as np
import math
import sys
import pylsl
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from PIL import Image
import cv2

print("Waiting for Color Stream")
streams = pylsl.resolve_byprop("type", "Color Image")
inlet = pylsl.StreamInlet(streams[0])
print(streams[0].channel_format())
print("Opened Color Stream")
sample, ts = inlet.pull_sample()
counter = 0
while sample != None:
    counter += 1
    width = 1280
    #width = 1920
    #width = 2560
    #width = 2048
    arr = np.array(sample).astype(np.uint8)
    arr = arr.reshape(int(len(sample)/(width*4)), width*4)
    b = arr[:,::4]
    g = arr[:, 1::4]
    r = arr[:,2::4]
    a = arr[:,3::4]
    arrays = [r, g, b, a]
    image = np.stack(arrays, axis=2)
    color = Image.fromarray(image, mode='RGBA')
    color.save("Kinect Images\color" + str(ts) + ".png", "PNG")
    print(len(sample))
    sample, ts = inlet.pull_sample(timeout=10)
print("Done with Color")
print(counter)
