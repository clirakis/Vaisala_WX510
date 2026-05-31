/**
 ******************************************************************
 *
 * Module Name : WXT510.h
 *
 * Author/Date : C.B. Lirakis / 30-May-26
 *
 * Description : class for WXT510 
 *               the wind sensor is broken on the unit I have. 
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
#ifndef __WXT510_hh_
#define __WXT510_hh_
#  include <string>
#  include "CObject.hh"

/// WXT510 documentation here. 
class WXT510 : public CObject
{
public:
    /// Default Constructor
    WXT510(void);
    /// Default destructor
    ~WXT510(void);
    /// WXT510 function
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
    bool Decode(const std::string& in);

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
    void Zero(void);

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
    void TimeTag(void);


    friend std::ostream& operator<<(std::ostream& output, const WXT510 &in);

private:
    struct timespec fTime;
    double fTemperature;
    double fPressure;
    double fHumidity;
    double fHeatT;
    double fHeatV;
    double fVSupply;
    double fVref;

    double fRain_accumulation;
    double fRain_duration;
    double fRain_intensity;
    double fHail_accumulation;
    double fHail_duration;
    double fHail_intensity;
    double fRain_PeakIntensity;
    double fHail_PeakIntensity;

    // Wind values
    double fWind_DirectionMin;
    double fWind_DirectionAvg;
    double fWind_DirectionMax;
    double fWind_SpeedMin;
    double fWind_SpeedAvg;
    double fWind_SpeedMax;

    /* Private decoding functions */

    /*!
     * R0 - wind, temperature, humidity, barometric pressure
     * overall composite message, this is configured as part of startup. 
     */
    bool DecodeR0(const std::string &in);
    /*!
     * Wind data only
     */
    bool DecodeR1(const std::string &in);
    /*! 
     * Temperature, humidity and pressure
     */
    bool DecodeR2(const std::string &in);
    /*!
     * Rain and Hail only
     */
    bool DecodeR3(const std::string &in);
    /*!
     * internal parameters
     */
    bool DecodeR5(const std::string &in);

    /*!
     * Decode the communications data
     */
    bool DecodeXU(std::string &in);

};
#endif
