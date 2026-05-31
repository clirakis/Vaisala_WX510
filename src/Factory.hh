/**
 ******************************************************************
 *
 * Module Name : Factory.hh
 *
 * Author/Date : C.B. Lirakis / 31-May-26
 *
 * Description : WXT510 Factory data
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
#ifndef __FACTORY_hh_
#define __FACTORY_hh_
#include <string>

/// Factory documentation here. 
class Factory {
public:
    /// Default Constructor
    Factory();
    /// Default destructor
    ~Factory();
    /// Factory function
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
    void Decode(std::string& in);

    friend std::ostream& operator<<(std::ostream& output, const Factory &in);

private:
    string fOptions, fOrderCode, fCalDate, fInfo, fSerial;
    double fVRef1, fVRef2;
};
#endif
