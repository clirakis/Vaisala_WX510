/********************************************************************
 *
 * Module Name : WeatherDisp.cpp
 *
 * Author/Date : CBL/01-Jun-26
 *
 * Description : Display Weather data
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *  https://linux.die.net/man/3/wgetch
 *
 ********************************************************************/
// System Includes
// System includes.
#include <iostream>
using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <ncurses.h>
#include <time.h>
#include <math.h>

// Local Includes
#include "tools.h"
#include "debug.h"
#include "WeatherDisp.hh"
#include "CLogger.hh"
#include "Weather.hh"

Weather_Display* Weather_Display::fWeather_Display;


static int CommandCol   = 2;
static const char *main_frame_window[] =
{
    "+----------------------------------------------------------------------------+",
    "|            Weather  (h)ome, (r)efresh                                        ",
    "+----------------------------------------------------------------------------+",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|  Messages -----------------------------------------------------------------+",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|                                                                            |",
    "|  Command  -----------------------------------------------------------------+",
    "|                                                                            |",
    "+----------------------------------------------------------------------------+"
};

#define MAINFRAMEWINSIZ sizeof(main_frame_window)/sizeof(main_frame_window[0])

static const char *pos_strings[] = {
             "    Weather Data                               ",
             "     Wind Speed :           Direction:         ",
             "    Temperature :            Humidity:         ",
             "            Rain:           Intensity:         ",
             "            Hail:           Intensity:         ",
             "                                               ",
             "                                               ",
             "                                               ",
	     "                                               ",
             "                                               ",
             "                                               ",
};

#define POS_STR_SIZE sizeof(pos_strings) / sizeof(pos_strings[0])


static const char *help_strings[] = {
             "                          Help                 ",
             "                                               ",
             "  d - toggle display data (Hex codes rec'd)    ",
             "                                               ",
             "  f - filename change, advance number          ",
             "  h - home                                     ",
             "                                               ",
             "  q - quit                                     ",
             "  r - refresh                                  ",
	     "                                               ",
             "  ? - help                                     ",
             "                                               ",
             "                                               ",
};

#define HELP_STR_SIZE sizeof(help_strings) / sizeof(help_strings[0])

/**
 ******************************************************************
 *
 * Function Name : Weather_Display constructor
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
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
Weather_Display::Weather_Display(void)
{
    SET_DEBUG_STACK;
    /* Store the this pointer */
    fWeather_Display = this;
    fRun = true;
    fDisplayData = true;

    initscr();
    start_color();
    init_pair( 1, COLOR_YELLOW, COLOR_BLUE);
    init_pair( 2, COLOR_BLUE  , COLOR_YELLOW);
    init_pair( 3, COLOR_BLUE  , COLOR_WHITE);

    /* Trying to supress garbage on the input side. */
    cbreak();
    noecho();
    nonl();
    keypad(stdscr, TRUE);

    /* 
     * newwin arguments - 
     * Number lines
     * ncolumns
     * begin_y
     * begin_x
     */
    fVin = newwin(24,80,0,0);
    refresh();
    fCurrentScreen = WEATHER_SCREEN;
    main_frame();

    WriteMsgToScreen("Start Display....");
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Weather_Display destructor
 *
 * Description : clean up all the ncurses stuff. 
 *
 * Inputs : none
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
Weather_Display::~Weather_Display(void)
{
    SET_DEBUG_STACK;
    WriteMsgToScreen("End Display....   ");
    delwin(fVin);
    endwin();
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : main_frame
 *
 * Description :  Paint the main frame of the system
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 22-Feb-22
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void Weather_Display::main_frame(void)
{
    SET_DEBUG_STACK;
    uint32_t i;
    int x = 0;
    int y = 0;

    werase(fVin);

    for (i = 0; i < MAINFRAMEWINSIZ; i++ )
    {
	wmove( fVin, x+i, y);
        wprintw(fVin, "%s", main_frame_window[i]);
    }

    switch (fCurrentScreen)
    {
    case WEATHER_SCREEN:
	x = 3;
	y = 2;
        for (i = 0; i < POS_STR_SIZE; i++ )
        {
            wmove( fVin, x+i, y);
	    wprintw( fVin, "%s",pos_strings[i]);
        }
        break;
    case HELP_SCREEN:
	x = 3;
	y = 2;
        for (i = 0; i < HELP_STR_SIZE; i++ )
        {
            wmove( fVin, x+i, y);
	    wprintw( fVin, "%s",help_strings[i]);
        }
	break;
    default:
	break;
    }

    /* set background color */
    wbkgd(fVin, COLOR_PAIR(1));

    /* push the output to the screen */
    wrefresh(fVin);
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : Update
 *
 * Description : This is called when an entire TSIP frame has been 
 * received and processed. This call will determine which screen is
 * active and parse the correct data. 
 *
 * Inputs : NONE
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void Weather_Display::Update(WXT510 *pW)
{
    SET_DEBUG_STACK;

    switch (fCurrentScreen)
    {
    case HELP_SCREEN:
	return;
	break;
    case WEATHER_SCREEN:
	display_all(pW);
	display_time(pW->Time().tv_sec);
	break;
    }

    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : 
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
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
void Weather_Display::display_time(time_t time)
{
    SET_DEBUG_STACK;
    int row = STATUS_AREA-1;
    int col = LEFT_AREA;
    char timestr[128];

    strftime( timestr, sizeof(timestr), "%F %H:%M:%S ", 
	      localtime(&time));
    wmove  (fVin, row, col);
    wprintw(fVin, "%s", timestr);

    SET_DEBUG_STACK;
}


/**
 ******************************************************************
 *
 * Function Name : display_all
 *
 * Description :
 *     
 *
 * Inputs : 
 *
 * Returns :
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
void Weather_Display::display_all(const WXT510 *pW)
{
    SET_DEBUG_STACK;
    int row, col;

    row = STATUS_AREA;
    col = LEFT_AREA;
  
    wmove  (fVin, row, col);
    wprintw(fVin, "%4.1f",pW->WindAverage());
    row++;
    
    wmove  (fVin, row, col);
    wprintw(fVin, "%5.2f", pW->Temperature());
    row++;
    
    wmove  (fVin, row, col);
    wprintw(fVin, "%4.2f", pW->RainAccumulation());
    row++;

    wmove  (fVin, row, col);
    wprintw(fVin, "%4.2f", pW->HailAccumulation());
    row++;

    row = STATUS_AREA;
    col = RIGHT_AREA;
    wmove  (fVin, row, col);
    wprintw(fVin, "%4.1f", pW->WindDir());
    row++;

    wmove  (fVin, row, col);
    wprintw(fVin, "%4.1f", pW->Humidity());
    row++;

    wmove  (fVin, row, col);
    wprintw(fVin, "%4.1f", pW->RainIntensity());
    row++;
    wmove  (fVin, row, col);
    wprintw(fVin, "%4.1f", pW->HailIntensity());
    row++;

    /* set background color */
    wbkgd(fVin, COLOR_PAIR(1));
    /* push the output to the screen */
    wrefresh(fVin);
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : 
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
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
void Weather_Display::WriteMsgToScreen(const char *s)
{
    static char msg[74];
    static int row = MESSAGE_AREA;

    SET_DEBUG_STACK;
    size_t n = strlen(s);
    if (n<1)
	return;

    if (n>sizeof(msg)) n = sizeof(msg);

    /* Clear out anything that is residual. */
    strncpy( msg, s, sizeof(msg));
    memset( &msg[n-1], 0x20, sizeof(msg)-n);

    wmove(fVin,row,2);
    row--;
    if (row == (STATUS_AREA + STATUS_HEIGHT)) row = MESSAGE_AREA;

    /* print something to the window. Standard printf like format */
    wprintw(fVin, "%s", msg);
    /* set background color */
    wbkgd(fVin, COLOR_PAIR(1));

    /* push the output to the screen */
    wrefresh(fVin);
    SET_DEBUG_STACK;
}

/**
 ******************************************************************
 *
 * Function Name : 
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
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
void Weather_Display::display_message (const char *fmt, ...)
{
    SET_DEBUG_STACK;
    va_list p;
    char s[256], c[80], *cp;
    static int once = FALSE;

    va_start(p, fmt);
    vsprintf(s, fmt, p);
    va_end(p);
    
    if ((!once) && (fCurrentScreen == 0))
    {
	once = TRUE;
	main_frame();
    }
    sprintf(c, "%-55.55s", s);
    if ((cp = strchr(s, '\n' )) != NULL)
    {
	*cp = '\0';
    }
    WriteMsgToScreen(s);
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : 
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
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
void Weather_Display::ClearCommandArea(void)
{
    SET_DEBUG_STACK;
    CommandCol = 2;
    wmove( fVin, COMMAND_AREA, CommandCol);
    wprintw(fVin, "                        ");

    /* set background color */
    wbkgd(fVin, COLOR_PAIR(1));

    /* push the output to the screen */
    wrefresh(fVin);
    SET_DEBUG_STACK;
}
/**
 ******************************************************************
 *
 * Function Name : 
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
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
void Weather_Display::DisplayCommandChar(unsigned char c)
{
    SET_DEBUG_STACK;
    wmove( fVin, COMMAND_AREA, CommandCol);    
    wprintw(fVin, "0x%X ", c);
    CommandCol += 5;

    /* set background color */
    wbkgd(fVin, COLOR_PAIR(1));

    /* push the output to the screen */
    wrefresh(fVin);
    SET_DEBUG_STACK;
} 

/**
 ******************************************************************
 *
 * Function Name : checkKeys
 *
 * Description : default screen is position status data. 
 * Changing such that the keys are checked based on which screen is active. 
 * Inputs :
 *
 * Returns :
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
bool Weather_Display::checkKeys(void)
{
    SET_DEBUG_STACK;
    bool rc = false;

    /* get a character from the window. */
    //char c = wgetch(fVin);
    int c = getch();    // This does less harm to the screen
    //int c = get_wch();

    if (((char)c) != '\0')
    {
	CLogger::GetThis()->LogTime("check keys %c\n", (char)c);
	switch (c)
	{
	case 0:
	    break;
	case 'd':
	case 'D':
	    fDisplayData = !fDisplayData;
	    break;
	case '?':
	    fCurrentScreen = HELP_SCREEN;
	    main_frame();
	    break;
	case 'f':
	case 'F':
	    // File name change requested. 
	    //On::GetThis()->UpdateFileName();
	    break;
	case 'h':
	case 'H':
	    fCurrentScreen = WEATHER_SCREEN;
	    main_frame();
	    break;
	case 'q':
	case 'Q':
	    /* QUIT */
	    rc = true;
	    break;
	case 'r':
	case 'R':
	    /* Repaint the screen */
	    main_frame();
	    break;
	default:
	    break;
	}
    }
    SET_DEBUG_STACK;
    return rc;
}

/**
 ******************************************************************
 *
 * Function Name : DisplayThread
 *
 * Description : A thread to update the display and check the 
 * keys in the display as necessary. 
 *
 * Inputs : void* arg - not used. 
 *
 * Returns : NONE
 *
 * Error Conditions : NONE
 * 
 * Unit Tested on: 6-Mar-19
 *
 * Unit Tested by: CBL
 *
 *
 *******************************************************************
 */
void* DisplayThread(void *arg)
{
    SET_DEBUG_STACK;
    int rv;
    const struct timespec sleeptime = {0L, 500000000L};
    Weather_Display *pDisp = Weather_Display::GetThis();
    CLogger::GetThis()->LogTime("Display thread starts.\n");

    while (pDisp->IsRunning())
    {
	/*
	 * Check to see if the user has requested
	 * special changes in the setup
	 */
	//rv = pDisp->checkKeys();
	rv = false; // Keeps picking up nonsense.
	if (rv)
	{
	    pDisp->WriteMsgToScreen("QUIT\0");
	    pDisp->Stop();
	    /* Command a graceful exit to the program. */
	    Weather::GetThis()->Stop(); 
	}
	else
	{
	    nanosleep( &sleeptime, NULL);
	    SET_DEBUG_STACK;
	    //pDisp->WriteMsgToScreen("UPDATE\0");
	}
    }
    CLogger::GetThis()->LogTime("Display thread stops.\n");
    SET_DEBUG_STACK;
    return NULL;
}

