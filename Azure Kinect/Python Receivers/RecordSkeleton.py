import math
import pylsl
import cv2
import csv

with open("Skeleton.csv", "w", newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["Order", "Tstamp", "x", "y", "z", "a1", "a2", "a3", "a4", "confidence"])
    print("Waiting for Skeleton Stream")
    streams = pylsl.resolve_byprop("type", "Skeleton Data")
    inlet = pylsl.StreamInlet(streams[0])
    print("Open Skeleton Stream")
    sample, ts = inlet.pull_sample()
    index = 1
    counter = 0
    while sample != None:
        counter += 1
        myList = [index]
        myList.append(str(ts))
        for elt in sample:
            myList.append(str(elt))
        writer.writerow(myList)
        index+=1
        sample, ts = inlet.pull_sample(timeout=10)
    print("Done with Skeleton")
    print(counter)
