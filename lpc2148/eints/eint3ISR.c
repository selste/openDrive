//
//  $Id: eint3ISR.c 158 2008-10-17 01:31:32Z jcw $
//  $Revision: 158 $
//  $Author: jcw $
//  $Date: 2008-10-16 21:31:32 -0400 (Thu, 16 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/eints/eint3ISR.c $
//

#include "FreeRTOS.h"
#ifdef CFG_UIP
#include "task.h"
#include "semphr.h"
#endif

#include "eint3ISR.h"

//
//  
//
#ifdef CFG_UIP
extern xSemaphoreHandle xENC28J60Semaphore;
#endif

//
//
//
static void eint3ISR_Handler (void)
{
#ifdef CFG_UIP
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
#endif

  SCB_EXTINT |= SCB_EXTINT_EINT3;

#ifdef CFG_UIP
  xSemaphoreGiveFromISR (xENC28J60Semaphore, &xHigherPriorityTaskWoken);

	VIC_VectAddr = (unsigned portLONG) 0;

  if (xHigherPriorityTaskWoken)
    portYIELD_FROM_ISR ();
#endif
}

void eint3ISR (void) __attribute__ ((naked));
void eint3ISR (void)
{
  portSAVE_CONTEXT ();
  eint3ISR_Handler ();
  portRESTORE_CONTEXT ();
}
