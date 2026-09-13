import serial # pyright: ignore[reportMissingModuleSource]
import time 

import sys
PORT = '/dev/tty.usbmodem2111303'
Baud=115200

ser=serial.Serial(PORT, Baud, timeout=2)
time.sleep(0.5) # let board settle
print (f" Conttest to { PORT}")

try:
    while True:
        line=ser.readline().decode('utf-8', errors='ignore').rstrip()
        if line:
            print(line)
        

except KeyboardInterrupt:
    print("\nStopped by user.")
    ser.close()
   