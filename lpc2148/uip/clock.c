//
//  $Id: clock.c 241 2008-10-29 10:59:45Z jcw $
//  $Revision: 241 $
//  $Author: jcw $
//  $Date: 2008-10-29 06:59:45 -0400 (Wed, 29 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uip/clock.c $
//

#include "FreeRTOS.h"
#include "task.h"

#include "clock.h"

clock_time_t clock_time (void)
{
  return xTaskGetTickCount ();
}
