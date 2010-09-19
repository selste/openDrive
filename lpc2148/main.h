//
//  $Id: main.h 343 2008-11-11 03:03:56Z jcw $
//  $Revision: 343 $
//  $Author: jcw $
//  $Date: 2008-11-10 22:03:56 -0500 (Mon, 10 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/main.h $
//

#ifndef _MAIN_H_
#define _MAIN_H_

//
//
//
#define __VERSION "1.44"

#define PROMPT "Demo>"

//
//
//
typedef enum
{
  TASKHANDLE_MONITOR,
#ifdef CFG_LED
  TASKHANDLE_GPS = 0,
#endif
#ifdef CFG_SENSORS
  TASKHANDLE_SENSORS,
#endif
#ifdef CFG_LED
  TASKHANDLE_LED,
#endif
#ifdef CFG_KBD
  TASKHANDLE_KBD,
#endif
#ifdef CFG_KBD
  TASKHANDLE_LCD,
#endif
#ifdef CFG_UIP
  TASKHANDLE_UIP,
#endif
  TASKHANDLE_LAST
}
taskHandle_e;

extern xTaskHandle taskHandles [TASKHANDLE_LAST];

#endif
