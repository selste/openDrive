//
//  $Id: leds.c 328 2008-11-09 05:00:23Z jcw $
//  $Revision: 328 $
//  $Author: jcw $
//  $Date: 2008-11-09 00:00:23 -0500 (Sun, 09 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/leds/leds.c $
//

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "../main.h"
#include "../cpu/cpu.h"
#include "leds.h"

//
//  If CFG_LED is not defined, we still compile the code, it's just not included.
//  Define TASKHANDLE_LED so we compile.  Normally this is defined in main.h,
//  but it's #ifndef'ed out so that we don't allocate a task handle slot for it
//  when LED support isn't included.
//
#ifndef CFG_LED
#define TASKHANDLE_LED 0
#endif

//
//
//
typedef struct
{
  int timeOn;
  int timeOff;
}
ledDutyCycles_t;

static ledDutyCycles_t ledDutyCycles [] = 
{
  { (200 / portTICK_RATE_MS), (800 / portTICK_RATE_MS) },
  { (400 / portTICK_RATE_MS), (600 / portTICK_RATE_MS) },
  { (600 / portTICK_RATE_MS), (400 / portTICK_RATE_MS) },
  { (800 / portTICK_RATE_MS), (200 / portTICK_RATE_MS) },
};

static xQueueHandle xLEDQueue = NULL;

//
//
//
void ledsInit (void)
{
  GPIO0_FIODIR |= (GPIO_IO_P10 | GPIO_IO_P11);
  GPIO0_FIOSET  = (GPIO_IO_P10 | GPIO_IO_P11);
}

void ledsLED1On (void)
{
  GPIO0_FIOCLR = GPIO_IO_P10;
}

void ledsLED1Off (void)
{
  GPIO0_FIOSET = GPIO_IO_P10;
}

//
//
//
portBASE_TYPE ledsSetDutyCycle (int adcValue)
{
  if (!xLEDQueue)
    return pdFALSE;

  return xQueueSend (xLEDQueue, &adcValue, (portTickType) 0);
}

//
//
//
static portTASK_FUNCTION (vLEDFlashTask, pvParameters __attribute__ ((unused)))
{
  portTickType ledTimeOn = 1;
  portTickType ledTimeOff = 1;
  portTickType lastTickTime;
  int dutyCycle = 0;

  //
  //  Create the queue, turn on LED and die if we can't
  //
  if (!xLEDQueue)
  {
    if ((xLEDQueue = xQueueCreate (1, sizeof (dutyCycle))) == 0)
    {
      ledsLED1On ();

      while (1)
        vTaskDelay (100);
    }
  }

  //
  //  Send ourselves a message to init the flash time
  //
  xQueueSend (xLEDQueue, &dutyCycle, (portTickType) 0);

  //
  //  We need to initialise lastTickTime prior to the first call to vTaskDelayUntil()
  //
  lastTickTime = xTaskGetTickCount ();

  for (;;)
  {
    vTaskDelayUntil (&lastTickTime, ledTimeOn);
    ledsLED1Off ();
    vTaskDelayUntil (&lastTickTime, ledTimeOff);
    ledsLED1On ();

    if (xQueueReceive (xLEDQueue, &dutyCycle, (portTickType) 0))
    {
      dutyCycle %= arrsizeof (ledDutyCycles);

      ledTimeOn  = ledDutyCycles [dutyCycle].timeOn;
      ledTimeOff = ledDutyCycles [dutyCycle].timeOff;
    }
  }
}

//
//
//
signed portBASE_TYPE ledsTaskStart (void)
{
  return xTaskCreate (vLEDFlashTask, (const signed portCHAR * const) "LED1", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2), &taskHandles [TASKHANDLE_LED]);
}

signed portBASE_TYPE ledsTaskStop (void)
{
  if (!taskHandles [TASKHANDLE_LED])
    return 0;
  
  vTaskDelete (taskHandles [TASKHANDLE_LED]);
  taskHandles [TASKHANDLE_LED] = NULL;
  ledsLED1Off ();
  return 1;
}
