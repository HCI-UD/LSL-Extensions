import numpy as np
import math
import pylsl
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from PIL import Image
import cv2
import csv
infos = pylsl.resolve_byprop("name", "Outlet")
info = infos[0]
stream = pylsl.StreamInlet(info)
for i in range(100):
    sample = stream.pull_sample()
    print(i)
