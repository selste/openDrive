//
//  $Id: leds_reuse.c 156 2008-10-17 01:28:41Z jcw $
//  $Revision: 156 $
//  $Author: jcw $
//  $Date: 2008-10-16 21:28:41 -0400 (Thu, 16 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/leds/leds_reuse.c $
//

//
//  This code was from the original demo package.  Although it is no longer 
//  used, it shows an excellent example of how to reuse task code with 
//  local variables.  
//
//  Each time the task is create with xTaskCreate, it increment the task
//  number, which it uses to calculate a flash rate and which LED to
//  flash.
//
#include "FreeRTOS.h"
#include "task.h"

#include "../cpu/cpu.h"
#include "leds.h"

//
//  Olimex board specific.  LEDs are on P0.10, P0.11
//
#define partstFIRST_IO			  ((unsigned portLONG) 0x400)
#define partstNUM_LEDS			  (2)

//
//
//
#define ledFLASH_RATE_BASE ((portTickType) 333)
static portBASE_TYPE uxFlashTaskNumber = 0;

//
//
//
void ledsToggleLED (unsigned portBASE_TYPE uxLED)
{
  unsigned portLONG ulLED = partstFIRST_IO;

  if (uxLED < partstNUM_LEDS)
  {
    ulLED <<= (unsigned portLONG) uxLED;

    if (GPIO0_FIOPIN & ulLED)
      GPIO0_FIOCLR = ulLED;
    else
      GPIO0_FIOSET = ulLED;			
  }	
}

//
//
//
portTASK_FUNCTION (vLEDFlashTask, pvParameters __attribute__ ((unused)))
{
  portTickType xFlashRate;
  portTickType xLastFlashTime;
  unsigned portBASE_TYPE uxLED;

  //
  // Calculate the LED and flash rate
  //
  portENTER_CRITICAL ();
  {
    uxLED = uxFlashTaskNumber;
    uxFlashTaskNumber++;
  }
  portEXIT_CRITICAL ();

  xFlashRate = ledFLASH_RATE_BASE + (ledFLASH_RATE_BASE * (portTickType) uxLED);
  xFlashRate /= portTICK_RATE_MS;

  //
  //  We will turn the LED on and off again in the delay period, so each delay is only half the total period
  //
  xFlashRate /= (portTickType) 2;

  //
  //  We need to initialise xLastFlashTime prior to the first call to vTaskDelayUntil()
  //
  xLastFlashTime = xTaskGetTickCount();

  for (;;)
  {
    vTaskDelayUntil (&xLastFlashTime, xFlashRate);
    ledsToggleLED (uxLED);
    vTaskDelayUntil (&xLastFlashTime, xFlashRate);
    ledsToggleLED (uxLED);
  }
}
