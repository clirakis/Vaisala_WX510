/********************************************************************
 *
 * Module Name : DataBuffer.cpp
 *
 * Author/Date : C.B. Lirakis / 23-May-21
 *
 * Description : Generic DataBuffer
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *
 ********************************************************************/
// System includes.

#include <iostream>
using namespace std;
#include <string>
#include <cmath>

// Local Includes.
#include "debug.h"
#include "DataBuffer.hh"

/**
 ******************************************************************
 *
 * Function Name : DataBuffer constructor
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
DataBuffer::DataBuffer (void)
{
    SET_DEBUG_STACK;
    fTime        = 0L;
    fTemperature = 0.0;
    fPressure    = 0.0;
    fHumidity    = 0.0;
    fRain        = 0.0;
}
DataBuffer::DataBuffer (const WXT510& in) 
{
    SET_DEBUG_STACK;
    fSelected    = kTEMPERATURE;
    Fill(in);
}

/**
 ******************************************************************
 *
 * Function Name : DataBuffer destructor
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
DataBuffer::~DataBuffer (void)
{
}


/**
 ******************************************************************
 *
 * Function Name : DataBuffer function
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void DataBuffer::Fill(const WXT510& in)
{
    SET_DEBUG_STACK;
    fTime        = in.Time().tv_sec;
    fTemperature = in.Temperature();
    fPressure    = in.Pressure();
    fHumidity    = in.Humidity();
    fRain        = in.RainAccumulation();
}

/**
 ******************************************************************
 *
 * Function Name : ostream for DataBuffer
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
ostream& operator<<(ostream& output, const DataBuffer &in)
{
    SET_DEBUG_STACK;

    output << in.fTime << ",";
    switch(in.fSelected)
    {
    case DataBuffer::kTEMPERATURE:
	output << in.fTemperature;
	break;
    case DataBuffer::kPRESSURE:
	output << in.fPressure;
	break;
    case DataBuffer::kHUMIDITY:
	output << in.fHumidity;
	break;
    case DataBuffer::kRAIN:
	output << in.fRain;
	break;
    }

    return output;
}
