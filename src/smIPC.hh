/**
 ******************************************************************
 *
 * Module Name : smIPC.hh
 *
 * Author/Date : C.B. Lirakis / 04-Jun-26
 *
 * Description : Inialize the IPC, in this case a shared memory IPC. 
 *
 * Restrictions/Limitations : NONE
 *
 * Change Descriptions : 
 *
 * Classification : Unclassified
 *
 * References : NONE
 *
 *******************************************************************
 */
#ifndef __SMIPC_hh_
#define __SMIPC_hh_
#   include "CObject.hh"
#   include "SharedMem2.hh"

class WX_IPC : public CObject 
{
public:
    /*! Constructor */
    WX_IPC(void);
    /*! Destructor */
    ~WX_IPC(void);
    /*! Send the data */
    void Update(const char* message);
    /*! provide shared memory for inbound commands. */
    void ProcessCommands(void);

private:
    /**
     * Shared memory for position data. R0 message
     */
    SharedMem2   *pSM_R0;
    SharedMem2   *pSM_Commands;        // A way to communicate with remote
};
#endif
