#include "debug.h"

#include "LPC17xx.h"
#include "LPC1768_bitdef.h"
#include "types.h"
#include "opendrive-config.h"

#include "FreeRTOS.h"
#include "task.h"
//#include "queue.h"
//#include "semphr.h"

#include "spi.h"
#include "spi_port.h"
#include "sdcard.h"


#include "TMC428.h"


void vSPI_StartTask( unsigned portBASE_TYPE uxPriority ) {
	static unsigned char ucParameterToPass;
	xTaskHandle xHandle;
    
    //xSSP0Port_Init(); //SDCard
    xSSP1Port_Init(); //TMC428

    //xSPIPort_Init();

	xTaskCreate( vSPI_Task, (signed portCHAR *) "SPI", SPI_STACK_SIZE, &ucParameterToPass, uxPriority, &xHandle );

	return;
}

void vSPI_Task( void * pvParameters ) {
	( void ) pvParameters;

    portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();



	while(1) {

		//SDCard_Init();
        TMC428_Init();
        //TMC428_Test();


		//delay task
		vTaskDelayUntil(&xLastWakeTime, ( 2000 / portTICK_RATE_MS ) );
	}
}


