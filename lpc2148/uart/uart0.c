//
//  $Id: uart0.c 234 2008-10-28 23:56:14Z jcw $
//  $Revision: 234 $
//  $Author: jcw $
//  $Date: 2008-10-28 19:56:14 -0400 (Tue, 28 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uart/uart0.c $
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
static xQueueHandle xRX0Queue; 
static xQueueHandle xTX0Queue; 

//
//  Communication flag between the interrupt service routine and serial API
//
static volatile portLONG *plTHREEmpty0;

//
//
//
signed portBASE_TYPE uart0Init (unsigned portLONG ulWantedBaud, unsigned portBASE_TYPE uxQueueLength)
{
  unsigned portLONG ulDivisor;
  unsigned portLONG ulWantedClock;
  static unsigned portLONG sulWantedBaud = 115200;
  static unsigned portBASE_TYPE suxQueueLength = 64;

  if (!ulWantedBaud)
    ulWantedBaud = sulWantedBaud;

  sulWantedBaud = ulWantedBaud;

  if (!uxQueueLength)
    uxQueueLength = suxQueueLength;

  suxQueueLength = uxQueueLength;

  uart0ISRCreateQueues (uxQueueLength, &xRX0Queue, &xTX0Queue, &plTHREEmpty0);

  if ((xRX0Queue == serINVALID_QUEUE) || (xTX0Queue == serINVALID_QUEUE) || (ulWantedBaud == (unsigned portLONG) 0))
    return 0;

  portENTER_CRITICAL ();

  {
    PCB_PINSEL0 = (PCB_PINSEL0 & ~(PCB_PINSEL0_P00_MASK | PCB_PINSEL0_P01_MASK)) | (PCB_PINSEL0_P00_TXD0 | PCB_PINSEL0_P01_RXD0);

    SCB_PCONP |= SCB_PCONP_PCUART0;

    //
    //  Setup the baud rate:  Calculate the divisor value
    //
    ulWantedClock = ulWantedBaud * serWANTED_CLOCK_SCALING;
    ulDivisor = configCPU_CLOCK_HZ / ulWantedClock;

    //
    //  Set the DLAB bit so we can access the divisor
    //
    UART0_LCR |= UART_LCR_DLAB;

    //
    //  Setup the divisor
    //
    UART0_DLL = (unsigned portCHAR) (ulDivisor & (unsigned portLONG) 0xff);
    ulDivisor >>= 8;
    UART0_DLM = (unsigned portCHAR) (ulDivisor & (unsigned portLONG) 0xff);

    //
    //  Turn on the FIFO's and clear the buffers
    //
    UART0_FCR = UART_FCR_EN | UART_FCR_CLR;

    //
    //  Setup transmission format
    //
    UART0_LCR = UART_LCR_NOPAR | UART_LCR_1STOP | UART_LCR_8BITS;

    //
    //  Setup the VIC for the UART
    //
    VIC_IntSelect &= ~VIC_IntSelect_UART0;
    VIC_VectAddr2 = (portLONG) uart0ISR;
    VIC_VectCntl2 = VIC_VectCntl_ENABLE | VIC_Channel_UART0;
    VIC_IntEnable = VIC_IntEnable_UART0;

    //
    //  Enable UART0 interrupts
    //
    UART0_IER |= UART_IER_EI;
  }

  portEXIT_CRITICAL ();

  return 1;
}

void uart0Deinit (void)
{
  VIC_IntEnClr = VIC_IntEnClr_UART0;

  SCB_PCONP &= ~SCB_PCONP_PCUART0;

  PCB_PINSEL0 = (PCB_PINSEL0 & ~(PCB_PINSEL0_P00_MASK | PCB_PINSEL0_P01_MASK)) | (PCB_PINSEL0_P00_GPIO | PCB_PINSEL0_P01_GPIO);
}

//
//
//
signed portBASE_TYPE uart0GetChar (signed portCHAR *pcRxedChar, portTickType xBlockTime)
{
  return xQueueReceive (xRX0Queue, pcRxedChar, xBlockTime) ? pdTRUE : pdFALSE;
}

//
//
//
signed portBASE_TYPE uart0PutChar (signed portCHAR cOutChar, portTickType xBlockTime)
{
  signed portBASE_TYPE xReturn = 0;

  portENTER_CRITICAL ();

  {
    //
    //  Is there space to write directly to the UART?
    //
    if (*plTHREEmpty0 == (portLONG) pdTRUE)
    {
      *plTHREEmpty0 = pdFALSE;
      UART0_THR = cOutChar;
      xReturn = pdPASS;
    }
    else 
    {
      //
      //  We cannot write directly to the UART, so queue the character.  Block for a maximum of 
      //  xBlockTime if there is no space in the queue.
      //
      xReturn = xQueueSend (xTX0Queue, &cOutChar, xBlockTime);

      //
      //  Depending on queue sizing and task prioritisation:  While we were blocked waiting to post 
      //  interrupts were not disabled.  It is possible that the serial ISR has emptied the Tx queue, 
      //  in which case we need to start the Tx off again.
      //
      if ((*plTHREEmpty0 == (portLONG) pdTRUE) && (xReturn == pdPASS))
      {
        xQueueReceive (xTX0Queue, &cOutChar, serNO_BLOCK);
        *plTHREEmpty0 = pdFALSE;
        UART0_THR = cOutChar;
      }
    }
  }

  portEXIT_CRITICAL ();

  return xReturn;
}

//
//
//
void uart0GetRxQueue (xQueueHandle *qh)
{
  *qh = xRX0Queue;
}
