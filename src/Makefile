##################################################################
#
#	Makefile for WXT510 using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	30-May-26       CBL     Original
#
#
######################################################################
# Machine specific stuff
#
#
TARGET = WXT510
#
# Compile time resolution.
#
INCLUDE = -I$(DRIVE)/common/utility -I$(DRIVE)/common/iolib \
	-I/usr/include/hdf5/serial
LIBS = -lutility -lio -lhdf5_cpp -lhdf5
LIBS += -L$(HDF5LIB) -lconfig++


# Rules to make the object files depend on the sources.
SRC     = 
SRCCPP  = main.cpp Weather.cpp WXT510.cpp UserSignals.cpp \
	Factory.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = Weather.hh WXT510.hh UserSignals.hh Version.hh \
	Factory.hh	

# When we build all, what do we build?
all:      $(TARGET)

include $(DRIVE)/common/makefiles/makefile.inc


