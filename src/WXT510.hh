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
#  include <stdint.h>
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
    inline void SetAddress(uint8_t add) {fAddress = add;};

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
    inline uint8_t Address(void) const {return fAddress;};


    inline struct timespec Time(void) const {return fTime;};
    inline double Temperature(void) const {return fTemperature;}; // C
    inline double Pressure(void) const {return fPressure;};       // BAR
    inline double Humidity(void) const {return fHumidity;};       // Percent
    // mm
    inline double RainAccumulation(void) const {return fRain_accumulation;};
    // mm/hr
    inline double RainIntensity(void) const {return fRain_intensity;};
    // Seconds
    inline double RainDuration(void) const {return fRain_duration;};
    inline double HailAccumulation(void) const {return fHail_accumulation;};
    inline double HailIntensity(void) const {return fHail_intensity;};
    inline double HailDuration(void) const {return fHail_duration;};
    inline double HeaterTemperature(void) const {return fHeatT;};
    inline double HeaterVoltage(void) const {return fHeatV;};
    inline double WindAverage(void) const {return fWind_SpeedAvg;};
    inline double WindDir(void) const {return fWind_DirectionAvg;};


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

    uint8_t fAddress;

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
    bool DecodeXU(const std::string &in);

    /*!
     * Decode the General parameters
     */
    bool DecodeXF(const std::string &in);


};
#endif
