/**
 ******************************************************************
 *
 * Module Name : WeatherDisp.hh
 *
 * Author/Date : C.B. Lirakis / 24-Dec-15
 *
 * Description : display relevant data about GTOP GPS receiver
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * 19-Feb-22   CBL Made into class. 
 *
 * Classification : Unclassified
 *
 * References :
 *
 *******************************************************************
 */
#ifndef __WEATHERDISP_h__
#define __WEATHERDISP_h__
#include <time.h>
#include <stdint.h>
#include <ncurses.h>
#include "Weather.hh"

/* Screen Layout */
/* Row where various messages are displayed. */
#define TOP_BAR        2
#define STATUS_AREA    4
#define STATUS_HEIGHT 11
#define MESSAGE_AREA  20
#define COMMAND_AREA  22

/* Columns */
#define LEFT_AREA     21
#define RIGHT_AREA    LEFT_AREA+16
#define COL_THREE     RIGHT_AREA+16


/* Screen definitions. */
#define WEATHER_SCREEN     1
#define HELP_SCREEN         WEATHER_SCREEN+1

class WXT510;

class Weather_Display 
{
public:
    /**
     * Constructor
     */
    Weather_Display(void);

    /**
     * Desctuctor
     */
    ~Weather_Display(void);

    /** 
     * Is the display running?
     */
    inline bool IsRunning(void) const {return fRun;};

    /**
     * command the display thread to exit. 
     */
    inline void Stop(void) {fRun = false;};


    /**
     * paint the main frame on the terminal. 
     */
    void main_frame(void);


    /*! Parse the keys associated with the options screen */
    void ParseOptionsKeys(char c);
    void paint_options(int incol, 
		       unsigned char pos_code, unsigned char vel_code,
		       unsigned char time_code, unsigned char aux_code);
    void copy_options(unsigned char pos_code, unsigned char vel_code,
		      unsigned char time_code, unsigned char aux_code);


    /**
     * Call this to update the display when the frame is complete. 
     */
    void Update(WXT510* pW);

    void WriteMsgToScreen(const char *s);
    // Return true if time to end program. 
    bool  checkKeys(void);

    /** Access the this pointer. */
    static Weather_Display* GetThis(void) {return fWeather_Display;};


private:
    void start_display(void);
    void end_display(void);
    void display_message (const char *fmt, ...);

    /* Position screen stuff. */
    void display_all(const WXT510* pPW);
    void display_time(time_t gpstime);
    
    /* Mainpulate command area. */
    void DisplayCommandChar(unsigned char c);
    void ClearCommandArea(void);

    WINDOW *fVin = NULL;
    int fCurrentScreen;
    bool fRun;
    bool fDisplayData; 

    // Store this pointer. 
    static Weather_Display* fWeather_Display;
};

/*
 * Thread for maintaining the display.
 */
void* DisplayThread(void *arg);

#endif

