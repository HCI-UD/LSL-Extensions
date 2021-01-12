import numpy as np
import math
import pylsl
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from PIL import Image
import cv2

print("Waiting for Sound Stream")
streams = pylsl.resolve_byprop("type", "Audio")
inlet = pylsl.StreamInlet(streams[0])
print("Opened Sound Stream")
sample, ts = inlet.pull_sample()
while sample != None:
    sample, ts = inlet.pull_sample(timeout=10)
print("Done with Sound")
