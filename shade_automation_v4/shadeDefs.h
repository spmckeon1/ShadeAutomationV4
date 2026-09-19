#pragma once

#include "config.h"

//#define PROD_SYS
#define TEST_SYS

/*
  VERSIONING PROCESS
  4.0.3.2
  │ │ │ │
  │ │ │ └── build/revision
  │ │ └──── patch
  │ └────── feature/minor
  └──────── application generation

  The third version componet increments for each change that fixes existing behavior without 
  intentionally adding a new user-visible capability.

  The fourth version component increments for each revision that successfully completes development, 
  compilation, testing, and validation and is promoted to the test production system.
*/

#define APP_VERSION "4.0.0.1"

#ifdef WINDSHIELD_SHADES
  #define PG_ID "wshdShade"
  #ifdef PROD_SYS
    #define APPNAME "Windshield Shade Controller"                     // the name of this application
    #define APP_SOURCE_ID "wsSd"
    #define PAGE_HEADER "wshdShade"                                   // web page header
    #define PG_TITLE "wshdShade Ctrl"
    #define ACCESS_PT_NAME "WS_SD_Controller"                         // access point name
  #elif defined TEST_SYS
    #define APPNAME "TEST Windshield Shade Controller"                // the name of this application
    #define APP_SOURCE_ID "wsSd"
    #define PAGE_HEADER "TEST wshdShade"                              // web page header
    #define PG_TITLE "TEST wshdShade Ctrl"
    #define ACCESS_PT_NAME "TEST_WS_SD_Controller"                    // access point name
  #endif
#elif defined DRIVER_SHADES
  #define PG_ID "drvShade"
  #ifdef PROD_SYS
    #define APPNAME "Drivers Window Shade Controller"                     // the name of this application
    #define APP_SOURCE_ID "drvSd"
    #define PAGE_HEADER "drvShade"                                   // web page header
    #define PG_TITLE "drvShade Ctrl"
    #define ACCESS_PT_NAME "DRV_SD_Controller"                         // access point name
  #elif defined TEST_SYS
    #define APPNAME "TEST Drivers Window Shade Controller"                // the name of this application
    #define APP_SOURCE_ID "drvSd"
    #define PAGE_HEADER "TEST drvShade"                              // web page header
    #define PG_TITLE "TEST drvShade Ctrl"
    #define ACCESS_PT_NAME "DRV_SD_Controller"                    // access point name
  #endif

#elif defined PASSENGER_SHADES
  #define PG_ID "passShade"
  #ifdef PROD_SYS
    #define APPNAME "Passenger Window Shade Controller"                     // the name of this application
    #define APP_SOURCE_ID "psSd"
    #define PAGE_HEADER "passShade"                                   // web page header
    #define PG_TITLE "passShade Ctrl"
    #define ACCESS_PT_NAME "PS_SD_Controller"                         // access point name
  #elif defined TEST_SYS
    #define APPNAME "TEST Passenger Window Shade Controller"                // the name of this application
    #define APP_SOURCE_ID "psSd"
    #define PAGE_HEADER "TEST psShade"                              // web page header
    #define PG_TITLE "TEST psShade Ctrl"
    #define ACCESS_PT_NAME "PS_SD_Controller"                    // access point name
  #endif

#else 
	#error "A shade controller must be defined.  Please do this in the 'shadeDefs.h' file befor before contnuing."
#endif




#define UPLOAD_PG "UPLOAD"                                          // page type is 'upload'

  #define SD_EVT_TYPE "WS_SD"

// ROUTE DEFINITIONS
#define SD_CMD_ROUTE                "to/nr/shadeState"
#define SD_CFG_ROUTE                "to/nr/shadeOps/cfg"
#define SD_CFG_STATE_ROUTE          "to/nr/shadeOps/cfg/state"
#define PK_BK_TOPIC                 "drvSd/to/nr/pk/bk"
#define PK_BK_ROUTE                 "to/nr/ctrlOps/pk/bk/state"

