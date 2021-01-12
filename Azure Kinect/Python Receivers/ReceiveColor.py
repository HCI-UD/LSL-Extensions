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
while sample != None:
    width = 1280
    arr = np.array(sample).astype(np.uint8)
    arr = arr.reshape(int(len(sample)/(width*4)), width*4)
    b = arr[:,::4]
    g = arr[:, 1::4]
    r = arr[:,2::4]
    a = arr[:,3::4]
    arrays = [r, g, b, a]
    image = np.stack(arrays, axis=2)
    color = Image.fromarray(image, mode='RGBA')
    #color.save("Saved Images\color" + str(ts) + ".png", "PNG")
    print(len(sample))
    print(sample[0:10])
    print(sys.getsizeof(sample[0]))
    sample, ts = inlet.pull_sample(timeout=10)
print("Done with Color")
