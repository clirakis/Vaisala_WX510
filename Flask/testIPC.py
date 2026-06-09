import sys
import os
import time
from PySM.smWXT510 import smWXT510

def main():
    pR0   = smWXT510()
    if(len(sys.argv)>1):
        if(sys.argv[1] == 'D'):
            pR0.debug = True
    
    for i in range(20):
         pR0.Read()
         print("index ", i, " time ", pR0.LastUpdate_Time_tv_sec," value: ", pR0.R0)
         time.sleep(1)
         
         

if __name__ == "__main__":
    main()
