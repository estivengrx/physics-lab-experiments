"""
Code to connect Arduino with Python using the serial library. 
The code reads distance measurements from the Arduino and plots them using 
matplotlib.
"""

import serial
import time
import matplotlib.pyplot as plt

# Serial port configuration
esp = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
time.sleep(2)
esp.write(b'a')
distances = []

# Read 10 lines
for i in range(10):
    line = esp.readline().decode().strip()

    while line == "":
        line = esp.readline().decode().strip()

    # Extract only the number
    try:
        value = float(line)
        distances.append(value)
    except:
        print("error", line)

esp.close()
time_axis = list(range(1, len(distances) + 1))  # axis from 1 to 10 seconds
plt.figure(figsize=(8, 5))
plt.plot(time_axis, distances, marker='o', linestyle='-', color='blue')
plt.ylim(0, 300)
plt.title("Distance measured by the sensor (10 samples)")
plt.xlabel("Measurement number")
plt.ylabel("Distance (cm)")
plt.grid(True)

plt.show()