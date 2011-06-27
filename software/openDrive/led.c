#include "debug.h"
#include "types.h"
#include "opendrive-config.h"

#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/task.h"

#include "led.h"
#include "led_port.h"

static inline void vLED_Init(void) {
	vLEDPort_Init();
}

static inline void vLED_Toggle(uint32_t *counter) {
	vLEDPort_Toggle(counter);
}

// Function that creates a task.
void vStartLEDTask( unsigned portBASE_TYPE uxPriority ) {
	static unsigned char ucParameterToPass;
	xTaskHandle xHandle;
	
	vLED_Init();
	
	xTaskCreate( vLEDTask, (signed portCHAR *) "LED", LED_STACK_SIZE, 
		     &ucParameterToPass, uxPriority, &xHandle );
	
	return;
}


void vLEDTask( void * pvParameters ) {
	( void ) pvParameters;
	
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	
	uint32_t counter = 0;
	
	while(1) {
		vLED_Toggle(&counter);
		
		//delay task for 250ms
		vTaskDelayUntil(&xLastWakeTime, ( 250 / portTICK_RATE_MS ) );
	}
}
