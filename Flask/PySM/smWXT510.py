"""@smWXT510
  Interface using the POSIX interface of shared memory for attaching
  to the shared memory holding the filename where the data is logged.
  This inherits from the base class of
  SharedMem2.py.

     Modified  By   Reason
     --------  --   ------
     03-Jun-26 CBL  Original


  References:
  https://www.programiz.com/python-programming/inheritance

  Unit Tested: 

 ====================================================================
"""
from PySM.SharedMem2 import SharedMem2
import math
import numpy as np

class smWXT510(SharedMem2):
    def __init__(self):
        # 40 bytes used for base class
        self.maxsize = 128
        params = {'name':'R0', 'size': self.maxsize, 'server': False}

        # self is implied when using super.
        super().__init__(params)

        # The only thing in this mechanism
        self.__R0 = None

    def __del__(self):
        super().__del__()

    def Read(self):
        """
        Read the data and put it into the local structure. 
        """
        super().Read()
        # Need to find where the string resides. Should start here
        start = self.bytes + 16 
        print ('start: ', start, ' maxsize: ', self.maxsize)
        print ('str : ', self.inb[start:])
        count = 0
        while ( (count < self.maxsize) and (self.inb[start+count] != 0)):
            count = count + 1
        print(' count: ', count)
        self.R0 = str(self.inb[start:start+count])
        self.UnpackDone()
        
        print("Read:", len(self.R0), " ", self.R0)
        
    def Print(self):
        print('Filename: ', self.__Filename)

    def __str__(self):
        rep  = "R0 message: " + self.__Filename
        return rep
        
