/********************************************************************
 *
 * Module Name : UserPlot.cpp
 *
 * Author/Date : C.B. Lirakis / 23-May-21
 *
 * Description : Generic UserPlot
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
#include <fstream>

// Local Includes.
#include "debug.h"
#include "UserPlot.hh"
#include "CLogger.hh"

/**
 ******************************************************************
 *
 * Function Name : UserPlot constructor
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
UserPlot::UserPlot (size_t BufferSize)
{
    SET_DEBUG_STACK;
    fMaxSize = BufferSize;
    fIndex   = 0;
}

/**
 ******************************************************************
 *
 * Function Name : UserPlot destructor
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
UserPlot::~UserPlot (void)
{
    SET_DEBUG_STACK;
    // Clean up the buffer
    DataBuffer *old;
    for (auto it = fBuffer.begin(); it != fBuffer.end(); ++it)
    {
	old = (DataBuffer *) *it;
	delete old;
    }
}


/**
 ******************************************************************
 *
 * Function Name : UserPlot function
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
void UserPlot::Fill(const WXT510& in)
{
    SET_DEBUG_STACK;
    DataBuffer *val, *old;

    val = new DataBuffer(in);

    if (fBuffer.size() < fMaxSize)
    {
	fBuffer.push_back(val);
	fIndex++;
    }
    else
    {
	fIndex = fIndex%fMaxSize;
	// delete the old entry
	old = (DataBuffer*) fBuffer[fIndex];
	delete old;
	// Add the new entry
	fBuffer[fIndex] = val;
    }
}
/**
 ******************************************************************
 *
 * Function Name : UserPlot function
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
void UserPlot::MakeFile(DataBuffer::DataType type)
{
    SET_DEBUG_STACK;
    ofstream myplot("WXT510.PLT");
    DataBuffer *val;

    if(myplot.is_open())
    {
	for (auto it = fBuffer.begin(); it != fBuffer.end(); ++it)
	{
	    val = (DataBuffer *) *it;
	    switch(type)
	    {
	    case DataBuffer::kTEMPERATURE:
		myplot << val->Time() << "," << val->Temperature() << endl;
		break;
	    case DataBuffer::kPRESSURE:
		myplot << val->Time() << "," << val->Pressure() << endl;
		break;
	    case DataBuffer::kHUMIDITY:
		myplot << val->Time() << "," << val->Humidity() << endl;
		break;
	    case DataBuffer::kRAIN:
		myplot << val->Time() << "," << val->Rain() << endl;
		break;
	    }
	}
	myplot.close();
    }
    else
    {
	CLogger::GetThis()->LogTime("Could not make the PLT file.\n");
    }
}

