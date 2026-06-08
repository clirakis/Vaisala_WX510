import sys
import os
from PySM.smWXT510 import smWXT510

def doit():
    pR0   = smWXT510()
    
    for i in range(20):
         pR0.Read()
         print("index ", i, " value: ", pR0.R0)
         

main():
    doit()
