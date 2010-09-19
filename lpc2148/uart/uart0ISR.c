//
//  $Id: uart0ISR.c 89 2008-10-10 15:25:48Z jcw $
//  $Revision: 89 $
//  $Author: jcw $
//  $Date: 2008-10-10 11:25:48 -0400 (Fri, 10 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uart/uart0ISR.c $
//

//
//  Standard includes
//
#include <stdlib.h>

//
//  Scheduler includes
//
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

//
//  Demo application includes
//
#include "uart0.h"
#include "uart0ISR.h"

//
//  Constants to determine the ISR source
//
#define serSOURCE_THRE					  ((unsigned portCHAR) 0x02)
#define serSOURCE_RX_TIMEOUT			((unsigned portCHAR) 0x0c)
#define serSOURCE_ERROR					  ((unsigned portCHAR) 0x06)
#define serSOURCE_RX					    ((unsigned portCHAR) 0x04)
#define serINTERRUPT_SOURCE_MASK	((unsigned portCHAR) 0x0f)

//
//  Queues used to hold received characters, and characters waiting to be transmitted
//
static xQueueHandle xRX0Queue; 
static xQueueHandle xTX0Queue; 
static volatile portLONG lTHREEmpty0;

//
//
//
void uart0ISRCreateQueues (unsigned portBASE_TYPE uxQueueLength, xQueueHandle *pxRX0Queue, xQueueHandle *pxTX0Queue, portLONG volatile **pplTHREEmptyFlag)
{
  //
  //  Create the queues used to hold Rx and Tx characters
  //
  *pxRX0Queue = xRX0Queue = xQueueCreate (uxQueueLength, (unsigned portBASE_TYPE) sizeof (signed portCHAR));
  *pxTX0Queue = xTX0Queue = xQueueCreate (uxQueueLength + 1, (unsigned portBASE_TYPE) sizeof (signed portCHAR));

  //
  //  Initialise the THRE empty flag - and pass back a reference
  //
  lTHREEmpty0 = (portLONG) pdTRUE;
  *pplTHREEmptyFlag = &lTHREEmpty0;
}

//
//
//
static void uart0ISR_Handler (void)
{
  signed portCHAR cChar;
  portBASE_TYPE higherPriorityTaskWoken = pdFALSE;

  switch (UART0_IIR & serINTERRUPT_SOURCE_MASK)
  {
    //
    //  Not handling this, but clear the interrupt
    //
    case serSOURCE_ERROR :	
      {
        cChar = UART0_LSR;
      }
      break;

    //
    //  The THRE is empty.  If there is another character in the Tx queue, send it now,
    //  otherwise, no more characters, so indicate THRE is available
    //
    case serSOURCE_THRE	:	
      {
        if (xQueueReceiveFromISR (xTX0Queue, &cChar, &higherPriorityTaskWoken) == pdPASS)
          UART0_THR = cChar;
        else
          lTHREEmpty0 = pdTRUE;
      }
      break;

    //
    //  A character was received.  Place it in the queue of received characters
    //
    case serSOURCE_RX_TIMEOUT :
    case serSOURCE_RX	:	
      {
        cChar = UART0_RBR;

        xQueueSendFromISR (xRX0Queue, &cChar, &higherPriorityTaskWoken);
      }
      break;

    default	:
      break;
  }

  VIC_VectAddr = (unsigned portLONG) 0;

  if (higherPriorityTaskWoken)
    portYIELD_FROM_ISR ();
}

void uart0ISR (void) __attribute__ ((naked));
void uart0ISR (void)
{
  portSAVE_CONTEXT ();
  uart0ISR_Handler ();
  portRESTORE_CONTEXT ();
}
