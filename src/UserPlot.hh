/**
 ******************************************************************
 *
 * Module Name : UserPlot.h
 *
 * Author/Date : C.B. Lirakis / 07-Jul-26
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
#ifndef __USERPLOT_hh_
#define __USERPLOT_hh_
/// UserPlot documentation here. 
#include <stdint.h>
#include <vector>
#include "DataBuffer.hh"
#include "WXT510.hh"

class UserPlot {
public:
    /// Default Constructor
    UserPlot(uint32_t BufferSize);

    /// Default destructor
    ~UserPlot();
    /// UserPlot function
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
    void Fill(const WXT510& in);

    void MakeFile(DataBuffer::DataType type);

private:
    uint32_t                   fMaxSize;           // when to wrap. 
    uint32_t                   fIndex;             // current fill point
    std::vector<DataBuffer*>   fBuffer;
};
#endif
