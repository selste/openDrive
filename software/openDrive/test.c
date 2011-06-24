#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "types.h"
#include "opendrive-config.h"

/* Scheduler include files. */
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/task.h"
//#include "queue.h"



#include "test.h"
#include "uart.h"
//#include "pwm_port.h"
//#include "spi_port.h"



void vTaskCode( void * pvParameters ) {

	( void ) pvParameters;


    portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

    //extern xQueueHandle UART2_SendQueue;
    //portBASE_TYPE xStatus;
	
    xUART2_SendQueue_t Item;
	uint32_t test = 0;   
	 
	//uint32_t pwmvalue = 0;
    
    //vPWMPort_Init();
    //vPWMPort_SetMR(1000);
    //vPWMPort_StartPWM();

	//xSSP_CSMP_Init();


	while(1) {

		debug_printf("TestTask\n");

/*
		switch(test) {
			case 0:		SPICS_ALLHIGH; break;
			case 1:		SPICS_Y0LOW; break;
			case 2:		SPICS_Y1LOW; break;
			case 3:		SPICS_Y2LOW; break;
			case 4:		SPICS_Y3LOW; break;
			case 5:		SPICS_Y4LOW; break;
			case 6:		SPICS_Y5LOW; break;
			case 7:		SPICS_Y6LOW; break;
			case 8:		SPICS_Y7LOW; break;
		}

		test++;
		if(test > 8) {test = 0;}
*/		

		
		//vPWMPort_SetMR(pwmvalue);
        //pwmvalue += 340;
		//if(pwmvalue > 3400) {pwmvalue=0;}

/*		switch(pwmvalue) {
			case 0:		pwmvalue = 1700; break;
			case 1700:	pwmvalue = 3400; break;
			default:	pwmvalue = 0; break;
		}
*/

		


		if(test == 5) {
			strcpy(Item.ucValue, "MESSAGE1! MESSAGE1!");
			//strcpy(Item.ucSource, "TESTTASK");
			Item.uxSourceID = OD_SID_TESTTASK;

			xUART2_SendToQueue( &Item );
		}
		else if(test == 10) {
			test = 0;
			strcpy(Item.ucValue, "MESSAGE2! MESSAGE2!");
			Item.uxSourceID = OD_SID_TESTTASK;
			//strcpy(Item.ucSource, "TESTTASK");

			xUART2_SendToQueue( &Item );
		}
		test++;


		vTaskDelayUntil(&xLastWakeTime, ( 2000 / portTICK_RATE_MS ) );
	}
}


void vStartTestTask( unsigned portBASE_TYPE uxPriority ) {
	static unsigned char ucParameterToPass;
	xTaskHandle xHandle;

	portBASE_TYPE test;
	test = xTaskCreate( vTaskCode, (signed char *) "Test", testSTACK_SIZE, &ucParameterToPass, uxPriority, &xHandle );

	if(test == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) {
		debug_printf("errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY\n");		
	}

	return;
} 

void bla(void) {



}


