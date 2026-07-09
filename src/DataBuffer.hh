/**
 ******************************************************************
 *
 * Module Name : DataBuffer.h
 *
 * Author/Date : C.B. Lirakis / 07-Jul-27
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
#ifndef __DATABUFFER_hh_
#define __DATABUFFER_hh_
#include "WXT510.hh"

/// DataBuffer documentation here. 
class DataBuffer {
public:
    enum DataType {kTEMPERATURE=0, kPRESSURE, kHUMIDITY, kRAIN};

    /// Default Constructor
    DataBuffer();
    DataBuffer(const WXT510& in);
    /// Default destructor
    ~DataBuffer();
    /// DataBuffer function
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
    void Fill(const WXT510& input);

    inline void SelectField(DataType sel) {fSelected = sel;};

    friend std::ostream& operator<<(std::ostream& output,const DataBuffer &in);

    inline time_t Time(void)        {return fTime;}
    inline double Temperature(void) {return fTemperature;};
    inline double Pressure(void)    {return fPressure;};
    inline double Humidity(void)    {return fHumidity;};
    inline double Rain(void)        {return fRain;};

private:

    DataType fSelected;
    // Fields to store. 
    time_t fTime;
    double fTemperature;
    double fPressure;
    double fHumidity;
    double fRain; 
};
#endif
