#!/usr/bin/python3
 # ================================================================
 #
 # Module Name : wxt510.py
 #
 # Author/Date : C.B. Lirakis / 24-Jun-12
 #
 # Description : python interface to Vaisala weather station. 
 #
 # Restrictions/Limitations : 
 #
 # Change Descriptions :
 #
 # Classification : Unclassified
 #
 # References :
 #
 #
 #    Modified    By    Reason
 #    --------    --    ------
 #    05-Jul-13   CBL   Added in more of the actual features to run the
 #                      program. I think I will have this run as a 
 #                      client and connect to a server on Rod. 
 #
 # ================================================================
import sys 
import os
import string
import socket
import serial
import datetime

# Local import
from wxt510 import wxt510 as wxt510

HOST, PORT = "192.168.1.34", 9999
CR=0x0D
LF=0x0A

class Sensor:
    def __init__(self):
        """
        Initalize the system.
        1) Open the serial port, 19200 8N1
        2) Establish a connection with the server
        3) once we receive a full complement of data send it to the 
           server for logging etc. 
        @return a fully qualified self
        """
        self.Error = False
        self.wxt   = None
        self.Debug = False
        
        SerialPort = "/dev/ttyUSB0"
        now = datetime.datetime.now()
        self.logfile = open('sensor.log','w') 
        self.logfile.write("# ---------------------------------------------\n")
        self.logfile.write("# SYS-I-FILE: Sensor.py \n");
        self.logfile.write('# SYS-I-FILE: Logfile started: '+str(now)+'\n')
        
        try:
            # Set timeout to 2 seconds
            self.ser = serial.Serial( SerialPort, baudrate=19200, 
                                     bytesize=serial.EIGHTBITS,
                                     parity=serial.PARITY_NONE, 
                                     stopbits=serial.STOPBITS_ONE,
                                     timeout=1.0) 
            self.logfile.write("# SYS-I-OPEN: Serial port open. " + SerialPort + "\n")
            
        except:
            self.logfile.write("# SYS-E-OPEN: Error opening serial port.\n")
            self.Error = True
            return
        
        # create WXT object.
        self.wxt = wxt510()
        self.wxt.SetLogfile(self.logfile)
        self.wxt.Setup(self.ser)
        self.wxt.Configure(self.ser)
        

##        # Create a socket (SOCK_STREAM means a TCP socket)
##        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

##        try:
##            # Connect to server and send data
##            sock.connect((HOST, PORT))
##            self.logfile.write("# connected to server\n")
##        except:
##            self.logfile.write("# Could not connect to server\n")
        
    def Do(self):
        """ 
        The main loop of the program. 
        """
        print("Executing loop.")
        if(self.Debug):
            self.logfile.write("# Executing Loop\n")
        self.wxt.CheckSupervisor(self.ser)
        self.wxt.SupervisorECNo(self.ser)
        loop = True
        #for i in range(20):
        while(loop):
            instr = self.ser.readline()
            val = instr.decode('utf-8).strip()
            if (self.Debug):
                self.logfile.write(val+"\n")
            
            # FIXME, len always returns 3 even when empty. 
            if (len(val) > 3):
                #print str(i) + ' ' + val
                now = datetime.datetime.now()
                logstring = str(now) + '::' + val + '\n'
                self.logfile.write(logstring) 
                self.wxt.Decode(val, self.logfile)
        #sock.sendall(logstring)
        
    def __del__(self):
        """
        Close down and release resources allocated during program execution.
        """
        if(self.wxt != None):
            del self.wxt
        self.ser.close()
#        self.sock.close()
        self.logfile.write('# SYS-I-END: Program ends.\n')
        now = datetime.datetime.now()
        self.logfile.write('# SYS-I-ENDTIME ' + str(now) + '\n')
        self.logfile.close()

def main():

#    string1 = "0R5,Th=20.9C,Vh=15.0N,Vs=15.3V,Vr=3.522V"
#    string2 = "0R2,Ta=21.0C,Ua=60.8P,Pa=1007.2H"
#    string3 = "0R1,Dn=000#,Dm=000#,Dx=000#,Sn=0.0#,Sm=0.0#,Sx=0.0#"
#    sensor = wxt510()
#    sensor.Decode(string1)
#    sensor.Print()
    sensor = Sensor()
    if (not sensor.Error):
        sensor.Do()
    else:
        print("Error!")

if __name__ == '__main__':
    main()
