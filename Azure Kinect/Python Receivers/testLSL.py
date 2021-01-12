import pylsl

stream = pylsl.resolve_byprop("name", "Outlet")
print("1")
inlet = pylsl.StreamInlet(stream[0])
print(input())
sample = inlet.pull_sample()
print(sample[0])
string = input()
sample, time = inlet.pull_sample()
