//
//  $Id: gps.h 283 2008-11-06 01:22:02Z jcw $
//  $Revision: 283 $
//  $Author: jcw $
//  $Date: 2008-11-05 20:22:02 -0500 (Wed, 05 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/gps/gps.h $
//

#ifndef _GPS_H_
#define _GPS_H_

#include "FreeRTOS.h"
#include "semphr.h"

//
//
//
typedef struct gpsData_s
{
  unsigned char valid;
  double latitude;
  double longitude;
  double height;
  float groundSpeed;
  float trueCourse;
  unsigned char utcDay;
  unsigned char utcMonth;
  unsigned short utcYear;
  unsigned char utcHours;
  unsigned char utcMinutes;
  unsigned char utcSeconds;
  int restarts;
}
gpsData_t;

//
//
//
int gpsCopyData (gpsData_t *dst);
signed portBASE_TYPE gpsTaskStart (void);
signed portBASE_TYPE gpsTaskStop (void);

#endif
