import sys
import os
from PySM.smWXT510 import smWXT510

def main():
    pR0   = smWXT510()
    #pR0.debug = True
    
    for i in range(20):
         pR0.Read()
         print("index ", i, " time ", pR0.LastUpdate_Time_tv_sec," value: ", pR0.R0)
         

if __name__ == "__main__":
    main()
