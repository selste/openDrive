#include "main.h"
#include "led.h"
#include "lm3s_cmsis.h"

#include "FreeRTOS.h"
#include "task.h"


int main (void) {

	SystemInit();

	vStartLEDTask(mainLED_TASK_PRIORITY);

	vTaskStartScheduler();

	for( ;; );

	return(0);
}
