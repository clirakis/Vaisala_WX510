/**
 ******************************************************************
 *
 * Module Name : Weather.cpp
 *
 * Author/Date : C.B. Lirakis / 31-May-26
 *
 * Description : Combine serial interface with WXT510 and possibly 
 *               other sources. 
 *
 * Restrictions/Limitations : none
 *
 * Change Descriptions : 
 *
 * Classification : Unclassified
 *
 * References : 
 *              
 *
 *
 *******************************************************************
 */  
// System includes.
#include <iostream>
using namespace std;

#include <string>
#include <unistd.h>
#include <cstdlib>
#include <libconfig.h++>
using namespace libconfig;

/// Local Includes.
#include "Weather.hh"
#include "CLogger.hh"
#include "tools.h"
#include "debug.h"
#include "SerialIO.h"

Weather* Weather::fWeather;


/**
 ******************************************************************
 *
 * Function Name : Weather constructor
 *
 * Description : initialize CObject variables
 *
 * Inputs : currently none. 
 *
 * Returns : none
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
Weather::Weather(const char* ConfigFile) : WXT510()
{
    CLogger *Logger = CLogger::GetThis();
    char msg[128];

    /* Store the this pointer. */
    fWeather = this;
    SetName("Weather");
    SetError(); // No error.

    fRun            = true;
    fSerialIO       = NULL;
    fSerialPortName = "/dev/ttyUSB1";

    /* 
     * Set defaults for configuration file. 
     */
    fLogging = true;

    if(!ConfigFile)
    {
	SetError(ENO_FILE,__LINE__);
	return;
    }

    fConfigFileName = strdup(ConfigFile);
    if(!ReadConfiguration())
    {
	SetError(ECONFIG_READ_FAIL,__LINE__);
	return;
    }

    /* USER POST CONFIGURATION STUFF. */

    /* 
     * Open up the serial Port 
     * for the standard startup it is 19200, 8N1
     */

    fSerialIO = new SerialIO(fSerialPortName.c_str(), 
			     B19200, 
			     SerialIO::NONE,  
			     SerialIO::ModeCanonical, 
			     2, 2);

    if (fSerialIO->CheckError())
    {
	sprintf(msg, "Error opening serial port: %s\n", 
		fSerialPortName.c_str());
	Logger->LogError(__FILE__,__LINE__,'F',msg);
	SetError(SerialIO::BadOpen);
    }
    else
    {
	Logger->LogTime("Serial port: %s open.\n", fSerialPortName.c_str());
    }

    /* Do any configuration needed. */

    f5Logger = NULL;
    fn       = NULL;
    if (fLogging)
    {
	fn = new FileName("WXT510", "h5", One_Day);
	OpenLogFile();
    }

    Logger->Log("# Weather constructed.\n");

    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Weather Destructor
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
Weather::~Weather(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();

    // Do some other stuff as well. 
    if(!WriteConfiguration())
    {
	SetError(ECONFIG_WRITE_FAIL,__LINE__);
	Logger->LogError(__FILE__,__LINE__, 'W', 
			 "Failed to write config file.\n");
    }
    free(fConfigFileName);

    delete fSerialIO;

    /* Clean up */
    delete f5Logger;
    f5Logger = NULL;

    // Make sure all file streams are closed
    Logger->Log("# Weather closed.\n");
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Command
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
bool Weather::Command(const string& cmd)
{
    SET_DEBUG_STACK;
    if(Debug(0))
    {
	CLogger::GetThis()->LogTime(" Command %s\n", cmd.c_str());
    }
    string toSend = cmd + "\n";
    fSerialIO->Write((unsigned char *)toSend.c_str(), toSend.size());
    return true;
}
/**
 ******************************************************************
 *
 * Function Name : SetProtocol
 *
 * Description : Set the protocol for the serial line
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
void Weather::SetProtocol(const string& Protocol)
{
    SET_DEBUG_STACK;
    string cmd;

    if ((Protocol=="A") || (Protocol=="a") || (Protocol=="P") || (Protocol=="p") || (Protocol=="N") || (Protocol=="Q") || (Protocol=="S") || (Protocol=="R"))
    {
	cmd = "0XXU,M=" + Protocol;
	Command(cmd);
    }
}
/**
 ******************************************************************
 *
 * Function Name : SetAutomaticInterval
 *
 * Description : Set the interval at which the data is provided. 
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
void Weather::SetAutomaticInterval(uint32_t sec)
{
    SET_DEBUG_STACK;
    if(sec<3600)
    {
	string cmd("0XU,");
	cmd += "I" + to_string(sec);
	Command(cmd);
    }
}

/**
 ******************************************************************
 *
 * Function Name : Setup
 *
 * Description : setup the system the way I want it. 
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
bool Weather::Setup(void)
{
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();

    pLog->LogTime(" Setup --------------\n");

    pLog->LogTime(" Reset!\n");
    Command("0XZ");
    ResetMeasurement();
    sleep(1);

    SetProtocol("P");
    SetAutomaticInterval(5);
    QueryCommunication();
    QueryGeneral();

    return true;
}


/**
 ******************************************************************
 *
 * Function Name : Configure
 *
 * Description : How do we want our messages?
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
bool Weather::Configure(void)
{
    SET_DEBUG_STACK;
    CLogger *pLog = CLogger::GetThis();

    pLog->LogTime(" Configure --------------\n");
    /*
     *
     * Configure Wind Parameters, R1
     * R = messages, page 95 in manual
     * I = update interval seconds
     * A = Average time
     * U = Speed unit: M = m/s, K = km/h, S = mph, N = knots
     * D = Direction correction: -180 ... 180
     * N = NMEA wind formatter
     * F = Sampling rate: 1, 2, or 4 Hz
     *
     */
    // FIXME
    //self.Command("0WU,R=1111110011111100,I=1,A=30,U=M,F=4", serial)

    /*
     * Turn it off
     * self.Command("0WU,I=0", serial)
     */

    /*
     * Pressure/Temperature/Humidity - R2
     * [R] = fields to transmit - page 100
     * [I] = Update interval: 1 ... 3600 seconds
     * [P] = Pressure unit: H = hPa, P = Pascal, B = bar, M = mmHg, I = inHg
     * [T] = Temperature unit: C = Celsius, F = Fahrenheit
     *
     */
    //self.Command("0TU,R=1111000011110000,I=5,P=B,T=C", serial);

    /*
     * Precipitation - R3
     * [R] = fields to transmit - page 104
     * [I] = Update interval: 1 ... 3600 seconds. This interval is
     *      valid only if the [M] field is = T
     * [U] = Precipitation units: M = metric (accumulated rainfall in mm,
     *       Rain duration in s, Rain intensity in mm/h)
     *       I = imperial (the corresponding parameters in units
     *       in, s, in/h)
     * [S] = Units for surface hits:
     *       M = metric (accumulated hailfall in hits/cm 2 , Hail
     *           event duration in s, Hail intensity in hits/cm 2 h)
     *       I = imperial (the corresponding parameters in units
     *           hits/in 2 , s, hits/in 2 h), H = hits (hits, s, hits/h)
     *           Changing the unit resets the precipitation counter.
     * [M] = Autosend mode: R = precipitation on/off, C = tipping
     *       bucket, T = time based
     *               R = precipitation on/off: The transmitter sends a
     *                   precipitation message 10 seconds after the first
     *                   recognition of precipitation. Rain duration Rd
     *                   increases in 10 s steps. Precipitation has ended
     *                   when:
     *                   Ri = 0. This mode is used for indication of the
     *                   start and the end of the precipitation.
     *                   C = tipping bucket: The transmitter sends a
     *                       precipitation message at each unit increment
     *                       (0.1mm/0.01 in). This simulates conventional
     *                       tipping bucket method.
     *                   T = time based: Transmitter sends a precipitation
     *                       message in the intervals defined in the
     *                       [I] field.
     *                       However, in polled protocols the autosend mode
     *                       tipping bucket should not be used as in it the
     *                       resolution of the output is decreased
     *                       (quantized to tipping bucket tips).
     * [Z] = Counter reset: M = manual, A = automatic, Y = immediate
     *       Sets both rain/hail accumulation count and duration
     *       of the rain/hail event to zero.
     *       M = manual reset mode: The counter is reset with
     *       aXZRU command only, see Precipitation Counter
     *       Reset (aXZRU) on page 54.
     *       A = automatic reset mode: The counts are reset after
     *       each precipitation message whether in automatic
     *       mode or when polled.
     *       Y = immediate reset: The counts are reset
     *       immediately after receiving the command.
     */
    //self.Command("0RU,R=1111110011111100,I=2,U=M,S=M,M=R,Z=A", serial)

    /*
     * Supervisor data - R5
     * [R] - fields to send, page 109
     * [I] = Update interval: 1 ... 3600 seconds. When the
     *       heating is enabled the update interval is forced to 15
     *       seconds.
     * [S] = Error messaging: Y = enabled, N = disabled
     * [H] = Heating control enable: Y = enabled, N = disabled
     *       Heating enabled: The control between full and half
     *       heating power is on as described in Heating
     *       (Optional) on page 24.
     *       Heating disabled: Heating is off in all conditions.
     */
    //self.Command("0SU,R=1111000011110000,I=120,S=Y,H=Y", serial)


    return true;
}
/**
 ******************************************************************
 *
 * Function Name : Do
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
void Weather::Do(void)
{
    SET_DEBUG_STACK;
    uint32_t count = 0;
    char line[256];
    int32_t  rc;

    fRun = true;

    while(fRun)
    {
	memset(line, 0, sizeof(line));
	rc = fSerialIO->Read((unsigned char *)line, sizeof(line));
	if (rc>0)
	{
	    TimeTag();
	    // A few incomplete sentances to start with. 
	    cout << line << endl;
	    if(Decode(line))
	    {
		if(Debug(0))
		{
		    cout << "Decode succeeded. " << endl;
		    cout << *this << endl;
		}
	    }
	}
	sleep(1);
	count++;
    }
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : OpenLogFile
 *
 * Description : Open and manage the HDF5 log file
 *
 * Inputs : none
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
bool Weather::OpenLogFile(void)
{
    SET_DEBUG_STACK;
#if 0
    // USER TO FILL IN.
    const char *Names = "Time:Lat:Lon:Z:NSV:PDOP:HDOP:VDOP:TDOP:VE:VN:VZ";
    CLogger *pLogger = CLogger::GetThis();
    /* Give me a file name.  */
    const char* name = fn->GetUniqueName();
    fn->NewUpdateTime();
    SET_DEBUG_STACK;

    f5Logger = new H5Logger(name,"Main Logger Dataset", NVar, false);
    if (f5Logger->CheckError())
    {
	pLogger->Log("# Failed to open H5 log file: %s\n", name);
	delete f5Logger;
	f5Logger = NULL;
	return false;
    }
    f5Logger->WriteDataTags(Names);

    /* Log that this was done in the local text log file. */
    time_t now;
    char   msg[64];
    SET_DEBUG_STACK;
    time(&now);
    strftime (msg, sizeof(msg), "%m-%d-%y %H:%M:%S", gmtime(&now));
    pLogger->Log("# changed file name %s at %s\n", name, msg);

    /*
     * If the IPC is realized, put the current filename into it.
     */ 
    if (fIPC)
    {
	fIPC->UpdateFilename(name);
    }
    
    fChangeFile = false;
#endif
    return true;
}
/**
 ******************************************************************
 *
 * Function Name : ReadConfiguration
 *
 * Description : Open read the configuration file. 
 *
 * Inputs : none
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
bool Weather::ReadConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();
    ClearError(__LINE__);
    Config *pCFG = new Config();

    /*
     * Open the configuragtion file. 
     */
    try{
	pCFG->readFile(fConfigFileName);
    }
    catch( const FileIOException &fioex)
    {
	Logger->LogError(__FILE__,__LINE__,'F',
			 "I/O error while reading configuration file.\n");
	return false;
    }
    catch (const ParseException &pex)
    {
	Logger->Log("# Parse error at: %s : %d - %s\n",
		    pex.getFile(), pex.getLine(), pex.getError());
	return false;
    }


    /*
     * Start at the top. 
     */
    const Setting& root = pCFG->getRoot();

    // USER TO FILL IN
    // Output a list of all books in the inventory.
    try
    {
	int    Debug;
	int    address = 0;

	/*
	 * index into group Weather
	 */
	const Setting &MM = root["Weather"];
	MM.lookupValue("Logging",   fLogging);
	MM.lookupValue("Debug",     Debug);
	MM.lookupValue("SerPort",   fSerialPortName);
	MM.lookupValue("Address",   address);
	SetDebug(Debug);
	SetAddress(address);
    }
    catch(const SettingNotFoundException &nfex)
    {
	// Ignore.
    }

    delete pCFG;
    pCFG = 0;
    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : WriteConfigurationFile
 *
 * Description : Write out final configuration
 *
 * Inputs : none
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
bool Weather::WriteConfiguration(void)
{
    SET_DEBUG_STACK;
    CLogger *Logger = CLogger::GetThis();
    ClearError(__LINE__);
    Config *pCFG = new Config();

    Setting &root = pCFG->getRoot();

    // USER TO FILL IN
    // Add some settings to the configuration.
    Setting &MM = root.add("Weather", Setting::TypeGroup);
    MM.add("Debug",     Setting::TypeInt)     = 0;
    MM.add("Logging",   Setting::TypeBoolean) = fLogging;
    MM.add("SerPort",   Setting::TypeString)  = fSerialPortName;
    MM.add("Address",   Setting::TypeInt)     = (int) Address();

    // Write out the new configuration.
    try
    {
	pCFG->writeFile(fConfigFileName);
	Logger->Log("# New configuration successfully written to: %s\n",
		    fConfigFileName);

    }
    catch(const FileIOException &fioex)
    {
	Logger->Log("# I/O error while writing file: %s \n",
		    fConfigFileName);
	delete pCFG;
	return(false);
    }
    delete pCFG;

    SET_DEBUG_STACK;
    return true;
}
