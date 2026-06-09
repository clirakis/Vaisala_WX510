import sys
import os
import time
from PySM.smWXT510 import smWXT510
from WX.wxt510 import wxt510

def main():
    pR0   = smWXT510()
    pwxt  = wxt510()
    logfile = open('debug.log','w') 
    pwxt.SetLogfile(logfile)

    if(len(sys.argv)>1):
        if(sys.argv[1] == 'D'):
            pR0.debug = True
    
    for i in range(5):
        if(pR0.Read()):
            print(' new data ')
        val = pR0.R0
        print("index ", i, " time ", pR0.LastUpdate_Time_tv_sec," value: ", val)
        pwxt.Decode(val,logfile)

        time.sleep(1)
         
    logfile.close()

if __name__ == "__main__":
    main()
