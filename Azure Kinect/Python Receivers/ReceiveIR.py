import numpy as np
import math
import pylsl
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from PIL import Image
import cv2

print("Waiting for IR Stream")
streams = pylsl.resolve_byprop("type", "IR Image")
inlet = pylsl.StreamInlet(streams[0])
print("Opened IR Stream")
sample, ts = inlet.pull_sample()
i = 0
while sample != None and i < 80:
    width = 640
    arr = np.array(sample).astype(np.uint16)
    image = arr.reshape(int(len(sample)/width), width)
    modified = image.astype(np.int32)
    modified = np.interp(modified, (modified.min(), modified.max()), (0, 255)).astype(np.uint8)
    depth = Image.fromarray(modified, mode='L')
    depth.save("Kinect Images\IR" + str(ts) + ".png", "PNG")
    sample, ts = inlet.pull_sample(timeout=10)
    i += 1
print("Done with IR")
