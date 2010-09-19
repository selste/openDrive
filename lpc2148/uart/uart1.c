//
//  $Id: uart1.c 234 2008-10-28 23:56:14Z jcw $
//  $Revision: 234 $
//  $Author: jcw $
//  $Date: 2008-10-28 19:56:14 -0400 (Tue, 28 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uart/uart1.c $
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
#include "uart1.h"
#include "uart1ISR.h"

//
//  Constants to setup and access the UART
//
#define serWANTED_CLOCK_SCALING   ((unsigned portLONG) 16)

//
//  Constants to setup and access the VIC
//
#define serINVALID_QUEUE  ((xQueueHandle) 0)
#define serNO_BLOCK       ((portTickType) 0)

//
//  Queues used to hold received characters, and characters waiting to be transmitted
//
static xQueueHandle xRX1Queue; 
static xQueueHandle xTX1Queue; 

//
//  Communication flag between the interrupt service routine and serial API
//
static volatile portLONG *plTHREEmpty1;

//
//
//
signed portBASE_TYPE uart1Init (unsigned portLONG ulWantedBaud, unsigned portBASE_TYPE uxQueueLength)
{
  unsigned portLONG ulDivisor;
  unsigned portLONG ulWantedClock;
  static unsigned portLONG sulWantedBaud = 4800;
  static unsigned portBASE_TYPE suxQueueLength = 64;

  if (!ulWantedBaud)
    ulWantedBaud = sulWantedBaud;

  sulWantedBaud = ulWantedBaud;

  if (!uxQueueLength)
    uxQueueLength = suxQueueLength;

  suxQueueLength = uxQueueLength;

  uart1ISRCreateQueues (uxQueueLength, &xRX1Queue, &xTX1Queue, &plTHREEmpty1);

  if ((xRX1Queue == serINVALID_QUEUE) || (xTX1Queue == serINVALID_QUEUE) || (ulWantedBaud == (unsigned portLONG) 0))
    return 0;

  portENTER_CRITICAL ();

  {
    PCB_PINSEL0 = (PCB_PINSEL0 & ~(PCB_PINSEL0_P08_MASK | PCB_PINSEL0_P09_MASK)) | (PCB_PINSEL0_P08_TXD1 | PCB_PINSEL0_P09_RXD1);

    SCB_PCONP |= SCB_PCONP_PCUART1;

    //
    //  Setup the baud rate:  Calculate the divisor value
    //
    ulWantedClock = ulWantedBaud * serWANTED_CLOCK_SCALING;
    ulDivisor = configCPU_CLOCK_HZ / ulWantedClock;

    //
    //  Set the DLAB bit so we can access the divisor
    //
    UART1_LCR |= UART_LCR_DLAB;

    //
    //  Setup the divisor
    //
    UART1_DLL = (unsigned portCHAR) (ulDivisor & (unsigned portLONG) 0xff);
    ulDivisor >>= 8;
    UART1_DLM = (unsigned portCHAR) (ulDivisor & (unsigned portLONG) 0xff);

    //
    //  Turn on the FIFO's and clear the buffers
    //
    UART1_FCR = UART_FCR_EN | UART_FCR_CLR;

    //
    //  Setup transmission format
    //
    UART1_LCR = UART_LCR_NOPAR | UART_LCR_1STOP | UART_LCR_8BITS;

    //
    //  Setup the VIC for the UART
    //
    VIC_IntSelect &= ~VIC_IntSelect_UART1;
    VIC_VectAddr3 = (portLONG) uart1ISR;
    VIC_VectCntl3 = VIC_VectCntl_ENABLE | VIC_Channel_UART1;
    VIC_IntEnable = VIC_IntEnable_UART1;

    //
    //  Enable UART0 interrupts//
    //
    UART1_IER |= UART_IER_EI;
  }

  portEXIT_CRITICAL ();

  return 1;
}

void uart1Deinit (void)
{
  VIC_IntEnClr = VIC_IntEnClr_UART1;

  SCB_PCONP &= ~SCB_PCONP_PCUART1;

  PCB_PINSEL0 = (PCB_PINSEL0 & ~(PCB_PINSEL0_P08_MASK | PCB_PINSEL0_P09_MASK)) | (PCB_PINSEL0_P08_GPIO | PCB_PINSEL0_P09_GPIO);
}

//
//
//
signed portBASE_TYPE uart1GetChar (signed portCHAR *pcRxedChar, portTickType xBlockTime)
{
  return xQueueReceive (xRX1Queue, pcRxedChar, xBlockTime) ? pdTRUE : pdFALSE;
}

//
//
//
signed portBASE_TYPE uart1PutChar (signed portCHAR cOutChar, portTickType xBlockTime)
{
  signed portBASE_TYPE xReturn = 0;

  portENTER_CRITICAL ();

  {
    //
    //  Is there space to write directly to the UART?
    //
    if (*plTHREEmpty1 == (portLONG) pdTRUE)
    {
      *plTHREEmpty1 = pdFALSE;
      UART1_THR = cOutChar;
      xReturn = pdPASS;
    }
    else 
    {
      //
      //  We cannot write directly to the UART, so queue the character.  Block for a maximum of 
      //  xBlockTime if there is no space in the queue.
      //
      xReturn = xQueueSend (xTX1Queue, &cOutChar, xBlockTime);

      //
      //  Depending on queue sizing and task prioritisation:  While we were blocked waiting to post 
      //  interrupts were not disabled.  It is possible that the serial ISR has emptied the Tx queue, 
      //  in which case we need to start the Tx off again.
      //
      if ((*plTHREEmpty1 == (portLONG) pdTRUE) && (xReturn == pdPASS))
      {
        xQueueReceive (xTX1Queue, &cOutChar, serNO_BLOCK);
        *plTHREEmpty1 = pdFALSE;
        UART1_THR = cOutChar;
      }
    }
  }

  portEXIT_CRITICAL (); 

  return xReturn;
}

//
//
//
void uart1GetRxQueue (xQueueHandle *qh)
{
  *qh = xRX1Queue;
}
