//
//  $Id: sensors.h 348 2009-01-12 01:24:28Z jcw $
//  $Revision: 348 $
//  $Author: jcw $
//  $Date: 2009-01-11 20:24:28 -0500 (Sun, 11 Jan 2009) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/sensors/sensors.h $
//

#ifndef _SENSORS_H_
#define _SENSORS_H_

#include "FreeRTOS.h"
#include "semphr.h"

//
//
//
typedef struct sensorData_s
{
  int  sensorCount;
  int  adcChanges;
  int  adcValue;
  int  adcValueRaw;
  int  dacValue;
  int  dacDirection;
}
sensorData_t;

//
//
//
int sensorsCopyData (sensorData_t *dst);
signed portBASE_TYPE sensorsTaskStart (void);
signed portBASE_TYPE sensorsTaskStop (void);

#endif
