//
//  $Id: leds.h 283 2008-11-06 01:22:02Z jcw $
//  $Revision: 283 $
//  $Author: jcw $
//  $Date: 2008-11-05 20:22:02 -0500 (Wed, 05 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/leds/leds.h $
//

#ifndef _LEDS_H_
#define _LEDS_H_

#include "FreeRTOS.h"

//
//
//
void ledsInit (void);
void ledsLED1On (void);
void ledsLED1Off (void);
portBASE_TYPE ledsSetDutyCycle (int adcValue);
signed portBASE_TYPE ledsTaskStart (void);
signed portBASE_TYPE ledsTaskStop (void);

#endif
