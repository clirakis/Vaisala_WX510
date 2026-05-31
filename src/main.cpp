/**
 ******************************************************************
 *
 * Module Name : 
 *
 * Author/Date : C.B. Lirakis / 05-Mar-19
 *
 * Description :
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *
 *******************************************************************
 */
// System includes.
#include <iostream>
using namespace std;
#include <string>
#include <unistd.h>

/// Local Includes.
#include "debug.h"
#include "tools.h"
#include "CLogger.hh"
#include "UserSignals.hh"
#include "Version.hh"
#include "Weather.hh"

/** Control the verbosity of the program output via the bits shown. */
static unsigned int VerboseLevel = 0;

/** Pointer to the logger structure. */
static CLogger   *logger;

/**
 ******************************************************************
 *
 * Function Name : Help
 *
 * Description : provides user with help if needed.
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 *******************************************************************
 */
static void Help(void)
{
    SET_DEBUG_STACK;
    cout << "********************************************" << endl;
    cout << "* Weather program for WXT510.              *" << endl;
    cout << "* Built on "<< __DATE__ << " " << __TIME__ << "*" << endl;
    cout << "* Available options are :                  *" << endl;
    cout << "*                                          *" << endl;
    cout << "********************************************" << endl;
}
/**
 ******************************************************************
 *
 * Function Name :  ProcessCommandLineArgs
 *
 * Description : Loop over all command line arguments
 *               and parse them into useful data.
 *
 * Inputs : command line arguments. 
 *
 * Returns : none
 *
 * Error Conditions : none
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
static void
ProcessCommandLineArgs(int argc, char **argv)
{
    int option;
    SET_DEBUG_STACK;
    do
    {
        option = getopt( argc, argv, "f:hHv:");
        switch(option)
        {
        case 'f':
            break;
        case 'h':
        case 'H':
            Help();
	    Terminate(0);
        break;
	case 'v':
	case 'V':
	    VerboseLevel = atoi(optarg);
	    break;
       }
    } while(option != -1);
}
/**
 ******************************************************************
 *
 * Function Name : Initialize
 *
 * Description : Initialze the process
 *               - Setup traceback utility
 *               - Connect all signals to route through the terminate 
 *                 method
 *               - Perform any user initialization
 *
 * Inputs : none
 *
 * Returns : true on success. 
 *
 * Error Conditions : depends mostly on user code
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
static bool Initialize(void)
{
    SET_DEBUG_STACK;
    char   msg[32];
    double version;

    SetSignals();
    // User initialization goes here. 
    sprintf(msg, "%d.%d",MAJOR_VERSION, MINOR_VERSION);
    version = atof( msg);
    logger = new CLogger("WXT510.log", "WXT510", version);
    logger->SetVerbose(VerboseLevel);

    return true;
}

/**
 ******************************************************************
 *
 * Function Name : main
 *
 * Description : It all starts here:
 *               - Process any command line arguments
 *               - Do any necessary initialization as a result of that
 *               - Do the operations
 *               - Terminate and cleanup
 *
 * Inputs : command line arguments
 *
 * Returns : exit code
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
int main(int argc, char **argv)
{
    /* test strings */
    const string string1 = "0R5,Th=20.9C,Vh=15.0N,Vs=15.3V,Vr=3.522V";
    const string string2 = "0R2,Ta=21.0C,Ua=60.8P,Pa=1007.2H";
    const string string3 = "0R1,Dn=000#,Dm=000#,Dx=000#,Sn=0.0#,Sm=0.0#,Sx=0.0#";
    const string string4 = "0R0,Dm=000#,Sm=0.0#,Ta=9.8C,Ua=68.2P,Pa=1.0133B,Rc=0.36M,Th=8.7C,Vh=12.2N";

    ProcessCommandLineArgs(argc, argv);
    if (Initialize())
    {
	Weather *pModule = new Weather("WXT510.cfg");

	if (pModule->Error() == 0)
	{
	    pModule->TimeTag();
#if 0
	    cout << "R0 test" << endl;
	    pModule->Decode(string4);
	    cout << *pModule << endl;
	    cout << "R1 Test" << endl;
	    pModule->Zero();
	    pModule->Decode(string3);
	    cout << *pModule << endl;
	    cout << "R2 Test" << endl;
	    pModule->Zero();
	    pModule->Decode(string2);
	    cout << *pModule << endl;
	    cout << "R5 Test" << endl;
	    pModule->Zero();
	    pModule->Decode(string1);
	    cout << *pModule << endl;
#endif
	    pModule->Do();
	}

    }
    Terminate(0);
}
