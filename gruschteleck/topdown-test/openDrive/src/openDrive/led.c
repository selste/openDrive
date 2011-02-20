#include "FreeRTOS.h"
#include "task.h"

#include "led.h"
#include "led_port.h"


// Function that creates a task.
void vStartLEDTask( unsigned portBASE_TYPE uxPriority ) {
	static unsigned char ucParameterToPass;
	xTaskHandle xHandle;

    vLEDPort_Init();

	xTaskCreate( vLEDTask, (signed portCHAR *) "LED", LED_STACK_SIZE, &ucParameterToPass, uxPriority, &xHandle );

	return;
}


void vLEDTask( void * pvParameters ) {
	( void ) pvParameters;

    portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

    uint32_t counter = 0;

	while(1) {
		vLEDPort_Toggle(&counter);

		//delay task for 250ms
		vTaskDelayUntil(&xLastWakeTime, ( 250 / portTICK_RATE_MS ) );
	}
}
