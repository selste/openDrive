//
//  $Id: timer.c 324 2008-11-09 04:57:15Z jcw $
//  $Revision: 324 $
//  $Author: jcw $
//  $Date: 2008-11-08 23:57:15 -0500 (Sat, 08 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/timer/timer.c $
//

//
//  This timer code was contributed by Dave Madden
//

#include "FreeRTOS.h"

#include "timer.h"

void timerInit (void)
{
  SCB_PCONP |= SCB_PCONP_PCTIM1;
}
