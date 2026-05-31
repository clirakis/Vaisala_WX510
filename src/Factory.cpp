/********************************************************************
 *
 * Module Name : Factory.cpp
 *
 * Author/Date : C.B. Lirakis / 23-May-21
 *
 * Description : Generic Factory
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
#include "Factory.hh"
#include "debug.h"

/**
 ******************************************************************
 *
 * Function Name : Factory constructor
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
Factory::Factory (void)
{
    fOptions   = "NONE";
    fOrderCode = "NONE";
    fCalDate   = "NONE";
    fInfo      = "NONE";
    fSerial    = "NONE";
    fVRef1 = fVRef2 = 0.0;
}

/**
 ******************************************************************
 *
 * Function Name : Factory destructor
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
Factory::~Factory (void)
{
}


/**
 ******************************************************************
 *
 * Function Name : Factory function
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
void Factory::Decode(string &in)
{
    SET_DEBUG_STACK;
    /*
     * Decode XF response - this is the factory settings
     * Example response:
     * 0XF!0XF,f=11111111&11100010,o=AAC1DB1A,c=A263,i=HEL___,n=A3430012,2=2528,3=3512 <
     */

    // FIXME
}

/**
 ******************************************************************
 *
 * Function Name : Factory function
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
ostream& operator<<(ostream& output, const Factory &in)
{
    SET_DEBUG_STACK;

    output << "Factory" << endl
	   << "    Options: " << in.fOptions << endl
	   << "  OrderCode: " << in.fOrderCode << endl
	   << "    CalDate: " << in.fCalDate << endl
	   << "       Info: " << in.fInfo << endl
	   << "     Serial: " << in.fSerial << endl
	   << "      VRef1: " << in.fVRef1 << endl
	   << "      VRef2: " << in.fVRef1 << endl;

    return output;
}
