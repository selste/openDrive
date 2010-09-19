//
//  $Id: rtcISR.c 226 2008-10-26 03:19:05Z jcw $
//  $Revision: 226 $
//  $Author: jcw $
//  $Date: 2008-10-25 23:19:05 -0400 (Sat, 25 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/rtc/rtcISR.c $
//

#include <stdlib.h>

//
// Scheduler includes
//
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#if defined CFG_CONSOLE_USB
#include "../usbser/usbser.h"
#elif defined CFG_CONSOLE_UART0
#include "../uart/uart0.h"
#elif defined CFG_CONSOLE_UART1
#include "../uart/uart1.h"
#else
#error "Eeek!  No console devices defined!"
#endif
#include "rtcISR.h"
#include "rtc.h"

//
//
//
static xQueueHandle consoleQueue = NULL;

//
//
//
void rtcISRInit (void)
{
#if defined CFG_CONSOLE_USB
  usbserGetRxQueue (&consoleQueue);
#elif defined CFG_CONSOLE_UART0
  uart0GetRxQueue (&consoleQueue);
#elif defined CFG_CONSOLE_UART1
  uart1GetRxQueue (&consoleQueue);
#else
#error "Eeek!  No console devices defined!"
#endif
}

//
//
//
static void rtcISR_Handler (void)
{
  portBASE_TYPE higherPriorityTaskWoken = pdFALSE;

  RTC_CCR = (RTC_CCR_CLKEN | RTC_CCR_CLKSRC);
  SCB_PCONP |= SCB_PCONP_PCRTC;

  if (RTC_ILR & RTC_ILR_RTCCIF)
  {
    U8 c = 0xff;

    if (consoleQueue)
      xQueueSendFromISR (consoleQueue, &c, &higherPriorityTaskWoken);

    RTC_ILR = RTC_ILR_RTCCIF;
  }

  if (RTC_ILR & RTC_ILR_RTCALF)
  {
    U8 c = 0xfe;

    if (consoleQueue)
      xQueueSendFromISR (consoleQueue, &c, &higherPriorityTaskWoken);

    RTC_ILR = RTC_ILR_RTCALF;
  }

  VIC_VectAddr = (unsigned portLONG) 0;

  RTC_CCR = (RTC_CCR_CLKEN | RTC_CCR_CLKSRC);
  SCB_PCONP &= ~SCB_PCONP_PCRTC;

  if (higherPriorityTaskWoken)
    portYIELD_FROM_ISR ();
}

void rtcISR (void) __attribute__ ((naked));
void rtcISR (void)
{
  portSAVE_CONTEXT ();
  rtcISR_Handler ();
  portRESTORE_CONTEXT ();
}
