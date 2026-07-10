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
#include "WeatherDisp.hh"
#include "smIPC.hh"
#include "UserPlot.hh"

Weather* Weather::fWeather;

/**
 * thread control for the display if selected. 
 */
static pthread_t d_thread;
const int NVar = 14;

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
    fPDisplay       = NULL;
    fIPC            = NULL;
    fPlot           = NULL;

    /* 
     * Set defaults for configuration file. 
     */
    fLogging        = true;
    fUpdateInterval = 5; // Seconds

    if(!ConfigFile)
    {
	SetError(ENO_FILE,__LINE__);
	return;
    }

    fConfigFileName = ConfigFile;
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

    if(fDisplay)
    {
	/* If the user has requested the display feature, start it now. */
	/* create the display. */
	fPDisplay = new Weather_Display();
	if( pthread_create(&d_thread, NULL, DisplayThread, NULL) == 0)
	{
	    Logger->Log("# Display Thread successfully created.\n");
	}
	else
	{
	    SET_DEBUG_STACK;
	    /* It is not the end of the world if this fails. */
	    Logger->Log("# Dispaly Thread failed.\n");
	}
    }

    /* 
     * Create IPC to send data to anything, but in this
     * case the flask app to consume.
     */ 
    fIPC = new WX_IPC();
    if (fIPC->Error() != 0)
    {
	Logger->LogError(__FILE__, __LINE__,'W',
				     "Could not initialize IPC.");
	fIPC = NULL;
	SetError(-2); 
	return;
    }

    /* Initialize the plotting routine */
    fPlot = new UserPlot(16); /* make the buffer small initially for debug.*/

    /*
     * Do any setup specific to the instrument 
     */
    Setup();      // Perform reset 
    Configure();  // Configure which messages are to be included and how. 
    SetAutomaticInterval(fUpdateInterval);

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

    // Kill the display thread.
    if(fDisplay)
    {
	fPDisplay->Stop();
	delete fPDisplay;
	fPDisplay = NULL;
    }

    // Do some other stuff as well. 
    if(!WriteConfiguration())
    {
	SetError(ECONFIG_WRITE_FAIL,__LINE__);
	Logger->LogError(__FILE__,__LINE__, 'W', 
			 "Failed to write config file.\n");
    }
    delete fPlot;
    delete fIPC;
    delete fSerialIO;

    // Make sure all file streams are closed
    Logger->Log("# Weather closed.\n");

    /* Clean up */
    delete f5Logger;
    f5Logger = NULL;
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : ReadResponse
 *
 * Description : See if there is any data from the serial port 
 *               if there is, decode it. 
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
bool Weather::ReadResponse(void)
{
    SET_DEBUG_STACK;
    static size_t count = 0;
    char line[256];
    char tmp[32];
    int32_t  rc;

    memset(line, 0, sizeof(line));
    rc = fSerialIO->Read((unsigned char *)line, sizeof(line));
    if (rc>0)
    {
	TimeTag();
	if (Debug(1))
	{
	    CLogger::GetThis()->LogTime("%s", line);
	}
	/* put the line into shared memory for other consumers. */
	if(fIPC)
	{
	    fIPC->Update(line);
	}
	/* if the display is selected, show the data. */
	if(fPDisplay)
	{
	    fPDisplay->WriteMsgToScreen(line);
	}
	if(Decode(line))
	{
	    /* 
	     * Upon successful decode do the this and that and the other 
	     * things
	     */
	    if(fPDisplay)
	    {
		fPDisplay->Update(this);
		snprintf(tmp, sizeof(tmp),"Count %d\n", count);
		fPDisplay->WriteMsgToScreen(tmp);
	    }
	    if(fLogging)
	    {
		LogData();
	    }
	    /* and finally update the plot file database */
	    fPlot->Fill(*this);
	    /* DEBUG FIX ME LATER */
	    /* Every 256 times dump the file for debug purposes initially */
	    if (count == 0)
	    {
		DumpPlot(DataBuffer::kTEMPERATURE);
	    }
	    count = (count+1)%256;
	}
    }
    else
    {
	return false;
    }
    SET_DEBUG_STACK;
    return true;
}

/**
 ******************************************************************
 *
 * Function Name : Command
 *
 * Description : Send a command to the WXT510 - manage the serial data.
 *
 * Inputs : cmd - string to send
 *
 * Returns : true on success
 *
 * Error Conditions : None at the moment
 * 
 * Unit Tested on: 10-Jul-26
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
bool Weather::Command(const string& cmd)
{
    SET_DEBUG_STACK;

    string toSend = cmd + "\r\n";
    if(Debug(0))
    {
	CLogger::GetThis()->LogTime(" Command %s", toSend.c_str());
    }
    fSerialIO->Write((unsigned char *)toSend.c_str(), toSend.size());
    // See if there is any response to this command, many do not have returns. 
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

    SetProtocol("P");         // ASCII Automatic, see manualpage 63
    SetAutomaticInterval(5);  // 5 Seconds between messages. 
    QueryCommunication();     // Get the status of the setups. 
    QueryGeneral();
    QuerySetup();

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
    string cmd;
    char sfmt[128];

    pLog->LogTime(" Configure --------------\n");
    /*
     *
     * Configure Wind Parameters, R1
     * R = messages, page 111 in manual
     * I = update interval seconds
     * A = Average time
     * U = Speed unit: M = m/s, K = km/h, S = mph, N = knots
     * D = Direction correction: -180 ... 180
     * N = NMEA wind formatter
     * F = Sampling rate: 1, 2, or 4 Hz
     *
     * bits 1-8 control R1 message
     * bits 9-16 contorl composite message R0
     *
     * R Bits
     * Left to Right
     *  1 - Dn Direction minimum
     *  2 - Dm Direction average
     *  3 - Dx Direction maximum
     *  4 - Sn Speed minimum
     *  5 - Sm Speed average
     *  6 - Sx Speed maximum
     *  7 - Spare
     *  8 - Spare
     *  & delmiter
     *  9 - Dn Wind direction minimum
     * 10 - Dm Direction average
     * 11 - Dx Direction maximum
     * 12 - Sn Speed minimum
     * 13 - Sm Speed average
     * 14 - Sx Speed maximum
     * 15 - Spare
     * 16 - Spare 
     * Right most
     */
    snprintf(sfmt, sizeof(sfmt), "0WU,R=1111110011111100,I=%d,A=30,U=M,F=4",
	fUpdateInterval);
    //cmd = string("0WU,R=1111110011111100,I=1,A=30,U=M,F=4");
    Command(sfmt);

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
     * R fields
     *
     * Left to Right
     *  1 - Pa Air pressure
     *  2 - Ta Air temperature
     *  3 - Ua Air humidity
     *  4 - Spare
     *  5 - Spare
     *  6 - Spare
     *  7 - Spare
     *  8 - Spare
     *  & delmiter
     *  9 - Pa Air pressure
     * 10 - Ta Air temperature
     * 11 - Ua Air humidity
     * 12 - Spare
     * 13 - Spare
     * 14 - Spare
     * 15 - Spare
     * 16 - Spare 
     * Right most
     *
     */
    snprintf(sfmt, sizeof(sfmt),"0TU,R=1111000011110000,I=%d,P=B,T=C",
	     fUpdateInterval); 
    //cmd = string("0TU,R=1111000011110000,I=5,P=B,T=C");
    Command(sfmt);

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
     *
     *
     * R fields
     *
     * Left to Right
     *  1 - Rc Rain amount
     *  2 - Rd Rain duration
     *  3 - Ri Rain intensity
     *  4 - Hc Hail amount
     *  5 - Hd Hail duration
     *  6 - Hi Hail intensity
     *  7 - Rp Rain peak
     *  8 - Hp Hail peak
     *  & delmiter
     *  9 - Rc Rain amount
     * 10 - Rd Rain duration
     * 11 - Ri Rain intensity
     * 12 - Hc Hail amount
     * 13 - Hd Hail duration
     * 14 - Hi Hail intensity
     * 15 - Rp Rain peak
     * 16 - Hp Hail peak
     * Right most
     *
     */
    cmd = string("0RU,R=1111110011111100,I=120,U=M,S=M,M=R,Z=A");
    Command(cmd);

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
    cmd = string("0SU,R=1111000011110000,I=120,S=Y,H=Y");
    Command(cmd);

    return true;
}
/**
 ******************************************************************
 *
 * Function Name : LogData
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
void Weather::LogData(void)
{
    SET_DEBUG_STACK;
    double t = (double) Time().tv_nsec;
    t *= 1.0e-9;
    t += (double) Time().tv_sec;
    
    f5Logger->FillInternalVector(t,                    0);
    f5Logger->FillInternalVector(WindAverage(),        1);
    f5Logger->FillInternalVector(WindDir(),            2);
    f5Logger->FillInternalVector(Temperature(),        3);
    f5Logger->FillInternalVector(Humidity(),           4);
    f5Logger->FillInternalVector(Pressure(),           5);
    f5Logger->FillInternalVector(RainAccumulation(),   6);
    f5Logger->FillInternalVector(RainIntensity(),      7);
    f5Logger->FillInternalVector(RainDuration(),       8);
    f5Logger->FillInternalVector(HailAccumulation(),   9);
    f5Logger->FillInternalVector(HailIntensity(),     10);
    f5Logger->FillInternalVector(HailDuration(),      11);
    f5Logger->FillInternalVector(HeaterTemperature(), 12);
    f5Logger->FillInternalVector(HeaterVoltage(),     13);
    f5Logger->Fill();
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

    fRun = true;

    while(fRun)
    {
	if (fLogging)
	{
	    /* Check to see if the logging interval has rolled over. */
	    if (fn->ChangeNames())
	    {
		UpdateFileName();
	    }
	}
	ReadResponse();
	sleep(1);
	count++;
    }
    CLogger::GetThis()->LogTime("Loop Stops.\n");
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

    // USER TO FILL IN.
    const char *Names = "Time:WAvg:WDAvg:Temp:Hum:Pres:RAcc:RInt:RDur:HAcc:HInt:HDir:HTemp:HVolt";
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
#if 0
    /*
     * If the IPC is realized, put the current filename into it.
     */ 
    if (fIPC)
    {
	fIPC->UpdateFilename(name);
    }
#endif    
    fChangeFile = false;

    return true;
}
/**
 ******************************************************************
 *
 * Function Name : UpdateFileName
 *
 * Description : Flush and close current log file, update the name, 
 *               and reopen.
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
void Weather::UpdateFileName(void)
{
    SET_DEBUG_STACK;
    /*
     * flush and close existing file
     * get a new unique filename
     * reset the timer
     * and go!
     *
     * Check to see that logging is enabled. 
     */
    if(f5Logger)
    {
	// This will close and flush the existing logfile. 
	delete f5Logger;
	f5Logger = NULL;
	// Now reopen
	OpenLogFile();
    }
    SET_DEBUG_STACK;
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
	MM.lookupValue("Interval",  fUpdateInterval);
	MM.lookupValue("Display",   fDisplay);
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
    MM.add("Interval",  Setting::TypeInt)     = fUpdateInterval;
    MM.add("Display",   Setting::TypeBoolean) = fDisplay;

    // Write out the new configuration.
    try
    {
	pCFG->writeFile(fConfigFileName);
	Logger->Log("# New configuration successfully written to: %s\n",
		    fConfigFileName.c_str());

    }
    catch(const FileIOException &fioex)
    {
	Logger->Log("# I/O error while writing file: %s \n",
		    fConfigFileName.c_str());
	delete pCFG;
	return(false);
    }
    delete pCFG;

    SET_DEBUG_STACK;
    return true;
}
