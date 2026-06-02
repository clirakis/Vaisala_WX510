/********************************************************************
 *
 * Module Name : WXT510.cpp
 *
 * Author/Date : C.B. Lirakis / 23-May-21
 *
 * Description : Generic WXT510
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * References :
 * USER'S GUIDE Vaisala Weather Transmitter WXT510 M210906EN-C
 *
 ********************************************************************/
// System includes.

#include <iostream>
using namespace std;
#include <string>
#include <cmath>
#include <cstring>
#include <stdint.h>
#include <sstream>

// Local Includes.
#include "debug.h"
#include "WXT510.hh"
#include "CLogger.hh"

/**
 ******************************************************************
 *
 * Function Name : WXT510 constructor
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
WXT510::WXT510 (void) : CObject()
{
    SET_DEBUG_STACK;
    Zero();
}

/**
 ******************************************************************
 *
 * Function Name : WXT510 destructor
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Coditions :
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
WXT510::~WXT510 (void)
{
}


/**
 ******************************************************************
 *
 * Function Name : Zero
 *
 * Description : Zero all inputs
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void WXT510::Zero(void)
{
    SET_DEBUG_STACK;
    fTemperature        = 0.0;
    fPressure           = 0.0;
    fHumidity           = 0.0;
    fHeatT              = 0.0;
    fHeatV              = 0.0;
    fVSupply            = 0.0;
    fVref               = 0.0;

    fRain_accumulation  = 0.0;
    fRain_duration      = 0.0;
    fRain_intensity     = 0.0;
    fHail_accumulation  = 0.0;
    fHail_duration      = 0.0;
    fHail_intensity     = 0.0;
    fRain_PeakIntensity = 0.0;
    fHail_PeakIntensity = 0.0;

    fWind_DirectionMin  = 0.0;
    fWind_DirectionAvg  = 0.0;
    fWind_DirectionMax  = 0.0;
    fWind_SpeedMin      = 0.0;
    fWind_SpeedAvg      = 0.0;
    fWind_SpeedMax      = 0.0;
}

/**
 ******************************************************************
 *
 * Function Name : Decode 
 *
 * Description : Decode the string into its constiuents
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
bool WXT510::Decode(const string &in)
{
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();

    /*
     * Typical input string
     * 0R0,Dm=000#,Sm=0.0#,Ta=7.8C,Ua=74.0P,Pa=1.0135B,Rc=0.36M,Th=6.4C,Vh=12.2N
     * 
     * First character is id of talker, ignore.
     * second tells what type of message it is. 
     * third tells more about the message for decoding. In the case
     * of an 'R' message it is 0,1,2,3
     */
    string id  = in.substr(1,1);
    string id2 = in.substr(2,1);
    string toParse = in.substr(4);
    if (id == "R")
    {
	int type = stoi(id2);
	switch(type)
	{
	case 0:
	    DecodeR0(toParse);
	    break;
	case 1:
	    DecodeR1(toParse);
	    break;
	case 2:
	    DecodeR2(toParse);
	    break;
	case 3:
	    DecodeR3(toParse);
	    break;
	case 5:
	    DecodeR5(toParse);
	    break;
	default:
	    pLog->LogTime("Input not recognized: %s\n", in.c_str());
	    return false;
	    break;
	    
	}
    }
    else if (id == "X")
    {
	// Supervisor messages.
	if(id2 == "U")
	{
	    // response to query configuration
	    DecodeXU(toParse);
	}
	else if(id2=="F")
	{
	    DecodeXF(toParse);
	}
    }
    // FIXME, need to add additional messages. 
    return true;
}
/**
 ******************************************************************
 *
 * Function Name : DecodeR0
 *
 * Description : Decode composite message
 *
 * Inputs : string minus the R0 preamble. 
 *
 * Returns : true on success.
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 31-May-26
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool WXT510::DecodeR0(const string& in)
{
    SET_DEBUG_STACK;
    string token;
    istringstream  sstream(in);
    double value = 0.0;
    size_t N = 0;

    while(getline(sstream, token, ','))
    {
	if (token.find("#") != string::npos)
	{
	    // error in field, throw error
	    value = -999.99;
	}
	else if((N = token.find("=")) != string::npos)
	{
	    value = stof(token.substr(N+1));
	}
	else
	{
	    value = 0.0;
	}
	
	if (token.find( "Dm") != string::npos)
	{
	    // Average Wind Direction
	    fWind_DirectionAvg = value;
	} 
	else if (token.find( "Dx") != string::npos)
	{
	    // Wind direction maximum
            fWind_DirectionMax = value;
	} 
	else if (token.find( "Sm") != string::npos)
	{
	    // Wind speed average
	    fWind_DirectionAvg = value;
	} 
	else if (token.find( "Sx") != string::npos)
	{
	    // Wind Speed Maximum
	    fWind_DirectionMax = value;
	} 
	else if (token.find( "Ta") != string::npos)
	{
	    // Air temperature
	    fTemperature = stof(token.substr(token.find("=")+1));
	} 
	else if (token.find( "Ua") != string::npos)
	{
	    // Relative Humitiy.
	    fHumidity = stof(token.substr(token.find("=")+1));
	} 
	else if (token.find( "Pa") != string::npos)
	{
	    // Air Pressure
	    fPressure = stof(token.substr(token.find("=")+1));
	} 
	else if (token.find( "Rc") != string::npos)
	{
	    // Rain accumulation
	    fRain_accumulation = stof(token.substr(token.find("=")+1));
	} 
	else if (token.find( "Rd") != string::npos)
	{
	    // Rain duration
	    fRain_duration = stof(token.substr(token.find("=")+1));
	} 
	else if (token.find( "Ri") != string::npos)
	{
	    // Rain intensity
	    fRain_intensity = stof(token.substr(token.find("=")+1));
	} 
	else if (token.find( "Hc") != string::npos)
	{
	    // Hail accumulation
	    fHail_accumulation = stof(token.substr(token.find("=")+1));
	} 
	else if (token.find( "Hd") != string::npos)
	{
	    // Hail duration
	    fHail_duration = stof(token.substr(token.find("=")+1));
	} 
	else if (token.find( "Rp") != string::npos)
	{
	    // Rain peak intensity
	    fRain_PeakIntensity = stof(token.substr(token.find("=")+1));
	} 
	else if (token.find( "Hp") != string::npos)
	{
	    // Hail peak intensity
	    fHail_PeakIntensity = stof(token.substr(token.find("=")+1));
	} 
	else if (token.find( "Th") != string::npos)
	{
	    // Heating temperature
	    fHeatT = stof(token.substr(token.find("=")+1));
	} 
	else if (token.find( "Vh") != string::npos)
	{
	    // Heating voltage
	    fHeatV = stof(token.substr(token.find("=")+1));
	} 
	else if (token.find( "Vr") != string::npos)
	{
	    // reference voltage should be 3.5V
	    fVref = stof(token.substr(token.find("=")+1));
	} 
// 	else if (token.find( "Vs") != string::npos)
// 	{
// 	    // supply voltage FIXME is this a field?
// 	    //fHeatV = stof(token.substr(token.find("=")+1));
// 	} 
    }
    return true;
}
/**
 ******************************************************************
 *
 * Function Name : DecodeR1
 *
 * Description : Decode the R1, wind data, message. 
 *
 *      0R1,Dn=000#,Dm=000#,Dx=000#,Sn=0.0#,Sm=0.0#,Sx=0.0#
 *           Dn - Wind direction minimum degrees
 *           Dm - Wind direction average
 *           Dx - Wind direction maximum
 *           Sn - Wind speed minimum
 *           Sm - Wind speed average
 *           Sx - Wind speed maximum
 *
 *
 * Inputs : string to decode
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
bool WXT510::DecodeR1(const string &in)
{
    SET_DEBUG_STACK;
    string token;
    istringstream  sstream(in);
    double value;
    size_t N;

    while(getline(sstream, token, ','))
    {
	if (token.find("#") != string::npos)
	{
	    // error in field, set error
	    value = 999.99;
	}
	else if((N = token.find("=")) != string::npos)
	{
	    value = stof(token.substr(N+1));
	}
	else
	{
	    value = 0.0;
	}

	if (token.find( "Dm") != string::npos)
	{
	    // Average Wind Direction
	    fWind_DirectionAvg = value;
	} 
	else if (token.find( "Dn") != string::npos)
	{
	    // Minimum Wind Direction
	    fWind_DirectionMin = value;
	} 
	else if (token.find( "Dx") != string::npos)
	{
	    // Wind direction maximum
            fWind_DirectionMax = value;
	} 
	else if (token.find( "Sm") != string::npos)
	{
	    // Wind speed average
	    fWind_SpeedAvg = value;
	} 
	else if (token.find( "Sn") != string::npos)
	{
	    // Wind speed min
	    fWind_SpeedMin = value;
	} 
	else if (token.find( "Sx") != string::npos)
	{
	    // Wind Speed Maximum
	    fWind_SpeedMax = value;
	} 
    }
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : DecodeR2
 *
 * Description : Decode the R2, 
 *
 *
 * Inputs : string to decode
 *      0R2,Ta=21.0C,Ua=60.8P,Pa=1007.2H
 *        Ta - Air temperature
 *        Ua - Relative humidity
 *        Pa - Air pressure
 *
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 31-May-26
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool WXT510::DecodeR2(const string &in)
{
    SET_DEBUG_STACK;
    string token;
    istringstream  sstream(in);
    double value;
    size_t N;

    while(getline(sstream, token, ','))
    {
	if (token.find("#") != string::npos)
	{
	    // error in field, set error
	    value = 999.99;
	}
	else if((N = token.find("=")) != string::npos)
	{
	    value = stof(token.substr(N+1));
	}
	else
	{
	    value = 0.0;
	}

	if (token.find( "Ta") != string::npos)
	{
	    // Air Temperature (C)
	    fTemperature = value;
	} 
	else if (token.find( "Ua") != string::npos)
	{
	    // Relative Humidity
	    fHumidity = value;
	} 
	else if (token.find( "Pa") != string::npos)
	{
	    // Air Pressure (Bar)
            fPressure = value;
	} 
    }
    return true;
}
/**
 ******************************************************************
 *
 * Function Name : DecodeR3
 *
 * Description : Decode the R3, precipitation message 
 *     0R3,Rc=0.0M,Rd=0s,Ri=0.0M,Hc=0.0M,Hd=0s,Hi=0.0M,Rp=0.0M,Hp=0.0M<cr><lf>
 *
 * Inputs : string to decode
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
bool WXT510::DecodeR3(const string &in)
{
    SET_DEBUG_STACK;
    string token;
    istringstream  sstream(in);
    double value;
    size_t N;

    while(getline(sstream, token, ','))
    {
	if (token.find("#") != string::npos)
	{
	    // error in field, set error
	    value = 999.99;
	}
	else if((N = token.find("=")) != string::npos)
	{
	    value = stof(token.substr(N+1));
	}
	else
	{
	    value = 0.0;
	}

	if (token.find( "Rc") != string::npos)
	{
	    // Rain Accumulation
	    fRain_accumulation = value;
	} 
	else if (token.find( "Rd") != string::npos)
	{
	    // Rain duration
	    fRain_duration = value;
	} 
	else if (token.find( "Ri") != string::npos)
	{
	    // Intensity
            fRain_intensity = value;
	} 
	else if (token.find( "Rp") != string::npos)
	{
	    // peak intensity
            fRain_PeakIntensity = value;
	} 
	else if (token.find( "Hc") != string::npos)
	{
	    // 
            fHail_accumulation = value;
	} 
	else if (token.find( "Hd") != string::npos)
	{
	    // 
            fHail_duration = value;
	} 
	else if (token.find( "Hi") != string::npos)
	{
	    // 
            fHail_intensity = value;
	} 
	else if (token.find( "Hp") != string::npos)
	{
	    // 
            fHail_PeakIntensity = value;
	} 
    }
    return true;
}
/**
 ******************************************************************
 *
 * Function Name : DecodeR5
 *
 * Description : Decode the R5, supervisor message 
 *
 *
 * Inputs : string to decode
 *   0R5,Th=20.2C,Vh=13.7N,Vs=14.0V,Vr=3.497V
 *   0R5,Th=22.4C,Vh=15.0N,Vs=15.3V,Vr=3.522V
 *           
 *       Th - heating temperature
 *       Vh - heating voltage
 *       Vs - Supply voltage
 *       Vr - 3.5V reference voltage.
 *
 *
 * Returns : true on success. 
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 31-May-26
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool WXT510::DecodeR5(const string &in)
{
    SET_DEBUG_STACK;
    string token;
    istringstream  sstream(in);
    double value;
    size_t N;

    while(getline(sstream, token, ','))
    {
	if (token.find("#") != string::npos)
	{
	    // error in field, set error
	    value = 999.99;
	}
	else if((N = token.find("=")) != string::npos)
	{
	    value = stof(token.substr(N+1));
	}
	else
	{
	    value = 0.0;
	}

	if (token.find( "Th") != string::npos)
	{
	    // Heater temp
	    fHeatT = value;
	} 
	else if (token.find( "Vh") != string::npos)
	{
	    // Heating Voltage
	    fHeatV = value;
	} 
	else if (token.find( "Vs") != string::npos)
	{
	    // Supply Voltage
            fVSupply = value;
	} 
	else if (token.find( "Vr") != string::npos)
	{
	    // Reference Voltage
            fVref = value;
	} 
    }
    return true;
}
/**
 ******************************************************************
 *
 * Function Name : DecodeXU
 *
 * Description : decode configuration
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

bool WXT510::DecodeXU(const string& in)
{
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();
    string token;
    istringstream  sstream(in);
    size_t N = 0;
    string value;

    pLog->Log("# XU Response.\n");
    while(getline(sstream, token, ','))
    {
	// NOT AS STRAIGHTFORWARD!
	if((N = token.find("=")) != string::npos)
	{
	    value = token.substr(N+1);
	}
	else
	{
	    value = "0";
	}

	if (token.find("A=") != string::npos)
	{
	    fAddress = stoi(value);
	    pLog->Log("#     Address: %s\n", value.c_str());
	}
	else if (token.find("M=") != string::npos)
	{
	    /* 
	     * Protocol
	     * A - ASCII automatic
	     * a - ASCII automatic with CRC
	     * P - ASCII polled
	     * p - ASCII polled with CRC
	     * N - NMEA0183 v3.0 automatic
	     * Q - NMEA0183 v3.0 polled
	     * S - SDI-12 v1.3
	     * R - SDI-12 continious measurement
	     */
	    pLog->Log("#     Protocol %s\n", value.c_str());
	}
	else if (token.find("T=") != string::npos)
	{
	    pLog->Log("#     Test Parameter: %s\n", value.c_str());
	}
	else if (token.find("I=") != string::npos)
	{
	    pLog->Log("#     Repeat interval: %s\n", value.c_str());
	}
	else if (token.find("B=") != string::npos)
	{
	    pLog->Log("#     Baud Rate: %s\n", value.c_str());
	}
	else if (token.find("D=") != string::npos)
	{
	    pLog->Log("#     Data Bits: %s\n", value.c_str());
	}
	else if (token.find("P=") != string::npos)
	{
	    pLog->Log("#     Parity: %s\n", value.c_str());
	}
	else if (token.find("S=") != string::npos)
	{
	    pLog->Log("#     Stop bits: %s\n", value.c_str());
	}
	else if (token.find("L=") != string::npos)
	{
	    pLog->Log("#     RS-485 line delay(ms): %s\n", value.c_str());
	}
	else if (token.find("N=") != string::npos)
	{
	    pLog->Log("#     Device Name: %s\n", value.c_str());
	}
	else if (token.find("V=") != string::npos)
	{
	    pLog->Log("#     Software Version: %s\n", value.c_str());
	}
    }
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : DecodeXF
 *
 * Description : decode General Parameters
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

bool WXT510::DecodeXF(const string& in)
{
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();
    string token;
    istringstream  sstream(in);
    size_t N = 0;
    string value;

    pLog->Log("# XF Response.\n");
    while(getline(sstream, token, ','))
    {
	if((N = token.find("=")) != string::npos)
	{
	    value = token.substr(N+1);
	}
	else
	{
	    value = "0";
	}

	if (token.find("f=") != string::npos)
	{
	    fAddress = stoi(value);
	    pLog->Log("#     Factory options: %s\n", value.c_str());
	}
	else if (token.find("o=") != string::npos)
	{
	    pLog->Log("#     Order code: %s\n", value.c_str());
	}
	else if (token.find("i=") != string::npos)
	{
	    pLog->Log("#     Info: %s\n", value.c_str());
	}
	else if (token.find("n=") != string::npos)
	{
	    pLog->Log("#     Serial number: %s\n", value.c_str());
	}
	else if (token.find("2=") != string::npos)
	{
	    pLog->Log("#     2.5V Reference: %s\n", value.c_str());
	}
	else if (token.find("3=") != string::npos)
	{
	    pLog->Log("#     3.5V Reference: %s\n", value.c_str());
	}
    }
    return true;
}


/**
 ******************************************************************
 *
 * Function Name : WXT510 function
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
void WXT510::TimeTag(void)
{
    SET_DEBUG_STACK;
    clock_gettime(CLOCK_REALTIME, &fTime);
}
/**
 ******************************************************************
 *
 * Function Name : WXT510 function
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


/**
 ******************************************************************
 *
 * Function Name : WXT510 function
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
ostream& operator<<(ostream& output, const WXT510 &in)
{
    SET_DEBUG_STACK;
    char tmp[64], sns[16];
    struct tm *stamp = gmtime(&in.fTime.tv_sec);
    double ns = in.fTime.tv_nsec;
    ns *= 1.0e-9;

    strftime(tmp, sizeof(tmp), "%F %T", stamp); 
    snprintf(sns, sizeof(sns), "%.4f", ns);   //FIXME
    strncat(tmp, sns, sizeof(tmp)-strlen(tmp));
    output << "WXT510 " << tmp << endl
	   << "           Temperature: " << in.fTemperature << endl
	   << "        Pressure (Bar): " << in.fPressure << endl
	   << "  Relative Humidity(%): " << in.fHumidity << endl
	   << "  WIND                  " << endl
	   << "      Speed" << endl
	   << "                   Min: " << in.fWind_SpeedMin << endl
	   << "                   Avg: " << in.fWind_SpeedAvg << endl
	   << "                   Max: " << in.fWind_SpeedMax << endl
	   << "      Direction" << endl 
	   << "                   Min: " << in.fWind_DirectionMin << endl
	   << "                   Avg: " << in.fWind_DirectionAvg << endl
	   << "                   Max: " << in.fWind_DirectionMax << endl
	   << "  HUMIDITY" << endl
	   << "      Heater" << endl
	   << "           Temperature: " << in.fHeatT << endl
	   << "               Voltage: " << in.fHeatV << endl
	   << "                  VRef: " << in.fVref << endl 
	   << "  RAIN " << endl
	   << "          Accumulation: " << in.fRain_accumulation << endl
           << "              Duration: " << in.fRain_duration << endl
	   << "             Intensity: " << in. fRain_intensity << endl
	   << "        Peak intensity: " << in.fRain_PeakIntensity << endl
	   << "  HAIL " << endl
	   << "          Accumulation: " << in. fHail_accumulation << endl
	   << "              Duration: " << in.fHail_duration << endl
	   << "             Intensity: " << in.fHail_intensity << endl
	   << "        Peak intensity: " << in.fHail_PeakIntensity << endl;
    return output;
}
