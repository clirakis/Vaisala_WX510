/********************************************************************
 *
 * Module Name : smIPC.cpp
 *
 * Author/Date : C.B. Lirakis / 04-Jun-26
 *
 * Description : Shared memory IPC for WXT510 data
 *
 * Restrictions/Limitations : NONE
 *
 * Change Descriptions : 
 *
 * Classification : Unclassified
 *
 * References : NONE
 *
 ********************************************************************/
// System includes.

#include <iostream>
using namespace std;
#include <string>
#include <cmath>
#include <cstring>

// Local Includes.
#include "debug.h"
#include "CLogger.hh"
#include "SharedMem2.hh"     // class definition for shared segment. 
#include "smIPC.hh"

// 22-Feb-26 upped command size allocation to 512
const size_t kCommandSize  = 512;          // Bytes of command data.
const size_t kWeatherSize  = 128;
static char zerobuf[kCommandSize];

#define DEBUG_SM 0
/**
 ******************************************************************
 *
 * Function Name : IPC_Initialize
 *
 * Description : Create all necessary shared memory segments
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : none
 * 
 * Unit Tested on: 23-Aug-14
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
WX_IPC::WX_IPC(void) : CObject()
{
    SET_DEBUG_STACK;

    CLogger *plogger = CLogger::GetThis();

    SetName("WX_IPC");
    SetError(); // No error.
    SetDebug(0);

    plogger->LogCommentTimestamp("IPC Initialize, shared memory.");

    pSM_Commands      = NULL;
    pSM_R0            = NULL;

    memset(zerobuf, 0, sizeof(zerobuf));

    /* Just a character buffer */
    pSM_R0 = new SharedMem2("R0", kWeatherSize, true);
    if (pSM_R0->CheckError())
    {
	plogger->LogError(__FILE__, __LINE__, 'W',
			 "R0 data SM failed.");
	delete pSM_R0;
	pSM_R0 = 0;
	SET_DEBUG_STACK;
	SetError(-1);
    }
    else
    {
	plogger->Log("# R0 SM successfully created.\n");
    }



    pSM_Commands = new SharedMem2("WX_Commands", kCommandSize, true);
    if (pSM_Commands->CheckError())
    {
	plogger->Log("# %s %d Commands shared memory failed.\n", 
		    __FILE__,  __LINE__);
	delete pSM_Commands;
	pSM_Commands = NULL;
	SetError(-5);
    }
    else
    {
	plogger->Log("# %s %d Commands shared memory attached.\n",
		    __FILE__,  __LINE__);
	// Sometimes there is residual crap in the buffer that 
	// needs clearing
	pSM_Commands->PutData(zerobuf);
        pSM_Commands->PutData(0.0);
    }
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name :  ProcessCommands
 *
 * Description : Process any commmands that have been issued by
 * the parent process. 
 *
 * Inputs : gps - lassen gps structure.
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 * Unit Tested on: 15-No-25
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void WX_IPC::ProcessCommands(void)
{
    SET_DEBUG_STACK;
    CLogger *plogger  = CLogger::GetThis();
    if (pSM_Commands != NULL)
    {
        // number of bytes in buffer
	float available = pSM_Commands->GetData(); 
	char command[kCommandSize];
	memset(command, 0, sizeof(command));

	//plogger->LogTime("command loop, size: %f\n", available);
	if (available > 0.0)
	{
	    pSM_Commands->GetData(command);
	    // Null terminate
	    command[(int)available + 1] = 0;
	    plogger->Log("# Command received: %d %s\n", 
			 (int)available, command);
	    // Process approprately.
#if 0
	    if (strncmp( command, "CF", 2) == 0)
	    {
		plogger->Log("# DEBUG: Change Filename command\n");
		GTOP::GetThis()->UpdateFileName();
		// Now clear out the data buffer. 
		// Otherwise the last command will stick around. 
		pSM_Commands->PutData(zerobuf);
		pSM_Commands->PutData(0.0);
	    }
	    else if (strncmp( command, "GF",2) == 0)
	    {
		// get the current filename. 
		const char *filespec = pGTOP->Filespec();
		pSM_Commands->PutData(filespec);

	    }
	    else if (strncmp( command, "CM", 2) == 0)
	    {
		plogger->Log("# DEBUG: Marker command\n");
		// Marker in H5 file
		pGTOP->SetFlag(1);
	    }
#endif
	    available = 0.0; // Commands have been processed. 
	    memset( command, 0, kCommandSize);
	    pSM_Commands->PutData(available);
	}
    }
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name :  Update
 *
 * Description : Update all gps data in shared memory.
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 * Unit Tested on: 21-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void WX_IPC::Update(const char* message)
{
    SET_DEBUG_STACK;
    char toSend[kWeatherSize];
    if (pSM_R0)
    {
	SET_DEBUG_STACK;
	// Limit the size of the message
	memset( toSend, 0, sizeof(toSend));
	snprintf( toSend, sizeof(toSend),"%s", message);
	cout << " to send: " << toSend << endl;
        pSM_R0->PutData(toSend);
	ProcessCommands();
    }
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name :  WX_IPC destructor
 *
 * Description : clean up any allocated data 
 *
 * Inputs : NONE
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 * Unit Tested on: 15-Nov-25
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
WX_IPC::~WX_IPC(void)
{
    SET_DEBUG_STACK;
    delete pSM_R0;
    delete pSM_Commands;
    SET_DEBUG_STACK;
}


