/**
 ******************************************************************
 *
 * Module Name : Weather.cpp
 *
 * Author/Date : C.B. Lirakis / 05-Mar-19
 *
 * Description : Lassen control entry points. 
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
#include <cmath>
#include <csignal>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
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
 * Function Name : Do
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

	/*
	 * index into group Weather
	 */
	const Setting &MM = root["Weather"];
	MM.lookupValue("Logging",   fLogging);
	MM.lookupValue("Debug",     Debug);
	MM.lookupValue("SerPort",   fSerialPortName);
	SetDebug(Debug);
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
