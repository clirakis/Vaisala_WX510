import sys 
import os
import string
import socket
import serial
import datetime



def main(cmd):
    CR=0x0D
    LF=0x0A
    SerialPort = "/dev/ttyUSB0"

    
    try:
        # Set timeout to 2 seconds
        pser = serial.Serial( SerialPort, baudrate=19200, 
                              bytesize=serial.EIGHTBITS,
                              parity=serial.PARITY_NONE, 
                              stopbits=serial.STOPBITS_ONE,
                              timeout=1.0)
    
        print("Serial port open. ", SerialPort + "\n")
            
    except:
        print("Error opening serial port.\n")
        exit()

    #command = "0SU\r\n"
    command = "0" + cmd + "\r\n"
    pser.write(command.encode())
    line = pser.readline()
    print("Response: ", line)
    

if __name__ == '__main__':
    main(sys.argv[1])
