/**
 ******************************************************************
 *
 * Module Name : Weather.hh
 *
 * Author/Date : C.B. Lirakis / 22-Feb-22
 *
 * Description : Template for a main class
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
 *******************************************************************
 */
#ifndef __WEATHER_hh_
#define __WEATHER_hh_
#  include <string>
#  include "CObject.hh" // Base class with all kinds of intermediate
#  include "H5Logger.hh"
#  include "filename.hh"
#  include "WXT510.hh"
class Weather_Display;
class SerialIO;

class Weather : public WXT510
{
public:
    /** 
     * Build on CObject error codes. 
     */
    enum {ENO_FILE=1, ECONFIG_READ_FAIL, ECONFIG_WRITE_FAIL};
    /**
     * Constructor 
     * All inputs are in configuration file. 
     */
    Weather(const char *ConfigFile);

    /**
     * Destructor for Weather 
     */
    ~Weather(void);

    /*! Access the This pointer. */
    static Weather* GetThis(void) {return fWeather;};

    /**
     * Main Module DO
     * 
     */
    void Do(void);

    /**
     * Tell the program to stop. 
     */
    void Stop(void) {fRun=false;};


    /*!
     * Description: 
     *   
     *
     * Arguments:
     *   
     *
     * Returns:
     *
     * Errors:
     *
     */
    inline void QueryCommunication(void) {Command("0XU");};
 
    /*!
     * Description: 
     *   
     *
     * Arguments:
     *   
     *
     * Returns:
     *
     * Errors:
     *
     */
    inline void QueryGeneral(void) {Command("0XF");};

    inline void QuerySetup(void) {Command("0SU");};
 
    inline void Reset(void) {Command("0XZ");};
    inline void ResetPrecipitationCounter(void) {Command("0XZRU");};
    inline void ResetPrecipitationIntensity(void) {Command("0XZRI");};
    inline void ResetMeasurement(void) {Command("0XZM");};

    void SetProtocol(const string &in);

    bool Configure(void);

    void SetAutomaticInterval(uint32_t sec);

    bool ReadResponse(void);

    // To Do
    inline void SupervisorECNo(void) {Command("0SU,S=N");};
    inline void CheckSupervisor(void) {Command("0SU");};

    /**
     * Control bits - control verbosity of output
     */
    static const unsigned int kVerboseBasic    = 0x0001;
    static const unsigned int kVerboseMSG      = 0x0002;
    static const unsigned int kVerboseFrame    = 0x0010;
    static const unsigned int kVerbosePosition = 0x0020;
    static const unsigned int kVerboseHexDump  = 0x0040;
    static const unsigned int kVerboseCharDump = 0x0080;
    static const unsigned int kVerboseMax      = 0x8000;
 
private:

    bool fRun;
    /*!
     * Tool to manage the file name. 
     */
    FileName*    fn;          /*! File nameing utilities. */
    PreciseTime* fTimer;      /*! */
    bool         fChangeFile; /*! Tell the system to change the file name. */

    /*!
     * Logging tool, log data to HDF5 file.  
     */
    H5Logger*    f5Logger;

    /*! 
     * Configuration file name. 
     */
    std::string  fConfigFileName;

    /* Collection of configuration parameters. */
    bool         fLogging;       /*! Turn logging on. */

    std::string  fSerialPortName;
    SerialIO*    fSerialIO;
    int32_t      fUpdateInterval;

    bool             fDisplay;
    Weather_Display* fPDisplay;


    /* Private functions. ==============================  */

    /*!
     * Open the data logger. 
     */
    bool OpenLogFile(void);
    void LogData(void);

    bool Command(const string& cmd);


    /*!
     * Setup, the way I want it to respond
     */
    bool Setup(void);


    /*!
     * Read the configuration file. 
     */
    bool ReadConfiguration(void);
    /*!
     * Write the configuration file. 
     */
    bool WriteConfiguration(void);


    /*! The static 'this' pointer. */
    static Weather *fWeather;

};
#endif
