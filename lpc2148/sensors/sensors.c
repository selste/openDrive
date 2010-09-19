//
//  $Id: sensors.c 348 2009-01-12 01:24:28Z jcw $
//  $Revision: 348 $
//  $Author: jcw $
//  $Date: 2009-01-11 20:24:28 -0500 (Sun, 11 Jan 2009) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/sensors/sensors.c $
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "../main.h"
#include "../adc/adc.h"
#include "../dac/dac.h"
#include "../leds/leds.h"
#include "sensors.h"

//
//  If CFG_SENSORS is not defined, we still compile the code, it's just not
//  included.  Define TASKHANDLE_SENSORS so we compile.  Normally this is
//  defined in main.h, but it's #ifndef'ed out so that we don't allocate a task
//  handle slot for it when sensors support isn't included.
//
#ifndef CFG_SENSORS
#define TASKHANDLE_SENSORS 0
#endif

//
//
//
static sensorData_t sensorData;
static xSemaphoreHandle semaphore = NULL;

//
//  Return 1 if got a copy, 0 if not.
//
int sensorsCopyData (sensorData_t *dst)
{
  if (semaphore && (xSemaphoreTake (semaphore, 100 / portTICK_RATE_MS) == pdTRUE))
  {
    memcpy (dst, &sensorData, sizeof (sensorData));
    xSemaphoreGive (semaphore);
    return 1;
  }

  memset (dst, 0, sizeof (sensorData_t));
  return 0;
}

//
//
//
static portTASK_FUNCTION (vSensorsTask, pvParameters __attribute__ ((unused)))
{
  portTickType xTickCount;
  int adcValueRaw;
  int adcValue;
  int adcLastValue = -1;
  int adcChanged = -1;
  int dacValue = 0;
  int dacDirection = -16;

  memset (&sensorData, 0, sizeof (sensorData));

  if (!semaphore)
    vSemaphoreCreateBinary (semaphore);

  xTickCount = xTaskGetTickCount ();

  for (;;)
  {
    vTaskDelayUntil (&xTickCount, 100 / portTICK_RATE_MS);

    //
    //  Adjust the DAC value so we output a slow sine wave
    //
    if ((dacValue <= 0) || (dacValue >= (1024 - dacDirection)))
      dacDirection = 0 - dacDirection;

    dacSet (dacValue);
    dacValue += dacDirection;

    //
    //  Read the current ADC value, keep only top 2 bits.  If it changes,
    //  tell the LED task to change the blink rate.
    //
    if ((adcValue = ((adcValueRaw = adcRead0_3 ()) >> 8)) != adcLastValue)
    {
      ledsSetDutyCycle (adcValue);
      adcLastValue = adcValue;
      adcChanged++;
    }

    //
    //  Update the sensors data
    //
    if (xSemaphoreTake (semaphore, portMAX_DELAY) == pdTRUE)
    {
      sensorData.sensorCount++;
      sensorData.adcChanges = adcChanged;
      sensorData.adcValue = adcValue;
      sensorData.adcValueRaw = adcValueRaw;
      sensorData.dacValue = dacValue;
      sensorData.dacDirection = dacDirection;

      xSemaphoreGive (semaphore);
    }
  }
}

//
//
//
signed portBASE_TYPE sensorsTaskStart (void)
{
  return xTaskCreate (vSensorsTask,  (const signed portCHAR * const) "Sensors", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 2), &taskHandles [TASKHANDLE_SENSORS]);
}

signed portBASE_TYPE sensorsTaskStop (void)
{
  if (!taskHandles [TASKHANDLE_SENSORS])
    return 0;
  
  vTaskDelete (taskHandles [TASKHANDLE_SENSORS]);
  taskHandles [TASKHANDLE_SENSORS] = NULL;
  return 1;
}
