import numpy as np
import math
import pylsl
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from PIL import Image
import cv2


stream = pylsl.resolve_byprop("name", "Depth", timeout = 20)
inlet1 = pylsl.StreamInlet(stream[0])

stream = pylsl.resolve_byprop("name", "Infrared", timeout = 20)
inlet2 = pylsl.StreamInlet(stream[0])

depth, ts = inlet1.pull_sample()
infrared, depth = inlet2.pull_sample()
iwidth = infrared[0]
dwidth = depth[0]

depthIm = np.array(depth)
infraredIm = np.array(infrared)

depthIm = np.reshape(depthIm, (int(len(depth))/dwidth, dwidth))
infraredIm = np.reshape(
"""
fig = plt.figure()
ims = []
stream = pylsl.resolve_byprop("name", "IR", timeout = 20)
print(len(stream))
inlet1 = pylsl.StreamInlet(stream[0])
print(stream[0].type(), stream[0].channel_count(), stream[0].nominal_srate(), stream[0].channel_format(), stream[0].source_id())
#sample = inlet1.pull_sample()

stream = pylsl.resolve_byprop("name", "Depth", timeout = 20)
print(len(stream))
inlet2 = pylsl.StreamInlet(stream[0])
print(stream[0].type(), stream[0].channel_count(), stream[0].nominal_srate(), stream[0].channel_format(), stream[0].source_id())
"""
stream = pylsl.resolve_byprop("name", "Skeleton", timeout = 20)
inlet4 = pylsl.StreamInlet(stream[0])
print(stream[0].type(), stream[0].channel_count(), stream[0].nominal_srate(), stream[0].channel_format(), stream[0].source_id())

print("Streams open")
"""
sample = inlet3.pull_sample()
print("3")
sample = inlet2.pull_sample()
print("2")
sample = inlet1.pull_sample()
"""
sample = inlet4.pull_sample()
print("Streams inited")

for i in range(100):
    sample, ts = inlet4.pull_sample()
    if i % 10 == 0:
        print(sample)
"""
for i in range(100):
    sample, ts = inlet1.pull_sample()
    width = 320
    arr = np.array(sample).astype(np.uint16)
    image = arr.reshape(int(len(sample)/width), width)
    ims.append([image])
    #plt.imshow(image, cmap = 'rainbow') #inferno
    #plt.show()
    modified = image.astype(np.int32)
    ir = Image.fromarray(modified, mode='I')
    #ir.show()
    ir.save("images\infrared" + str(i) + ".png", "PNG")

    #string = input()
    #print(string)
    sample, ts = inlet2.pull_sample()
    width = 320
    arr = np.array(sample).astype(np.uint16)
    image = arr.reshape(int(len(sample)/width), width)
    ims.append([image])
    print(image.shape)
    #plt.imshow(image, cmap = 'gray')
    #plt.show()
    modified = image.astype(np.int32)
    depth = Image.fromarray(modified, mode='I')
    #depth.show()
    depth.save("images\depth" + str(i) + ".png", "PNG")    #string = input()
    #print(string)

    sample, ts = inlet3.pull_sample()
    width = 1280
    arr = np.array(sample).astype(np.uint8)
    arr = arr.reshape(int(len(sample)/(width*4)), width*4)
    b = arr[:,::4]
    g = arr[:,1::4]
    r = arr[:,2::4]
    a = arr[:,3::4]
    arrays = [r, g, b, a]
    #arrays = [b, g, r, a]
    image = np.stack(arrays, axis=2)
    ims.append([image])
    #plt.imshow(image)
    #plt.show()
    color = Image.fromarray(image, mode='RGBA')
    #color.show()
    color.save("images\color" + str(i) + ".png", "PNG")
    print("done " + str(i))
"""
