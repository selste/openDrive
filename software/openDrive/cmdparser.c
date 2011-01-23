#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

#include "debug.h"
#include "LPC17xx.h"
#include "LPC1768_bitdef.h"
#include "types.h"
#include "opendrive-config.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "cmdparser.h"
#include "uart.h"
#include "TMC428.h"

volatile xQueueHandle CMDParser_CMDQueue;

void vCMDParser_StartTask( unsigned portBASE_TYPE uxPriority ) {
	static unsigned char ucParameterToPass;
	xTaskHandle xHandle;

	CMDParser_CMDQueue = xQueueCreate(CMDParser_CMDQUEUE_ITEMS, sizeof(CMDParser_CMDQueue_t));	// create UART2 SendQueue

	portBASE_TYPE handler;
	if(xTaskCreate( vCMDParser_Task, (signed char *) "CMDParser", CMDParser_STACK_SIZE, &ucParameterToPass, uxPriority, &xHandle) == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) {

		debug_printf("errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY\n");		
	}

	return;
} 


void vCMDParser_Task( void * pvParameters ) {

	( void ) pvParameters;
	CMDParser_CMDQueue_t QueueItem;
	//unsigned char ResponseMsg[OD_CMDLENGTH];
    xUART2_SendQueue_t Response;
    Response.uxSourceID = OD_SID_CMDPARSER;

	//unsigned char tmp[OD_CMDLENGTH];

	while(1) {
		//Sleep until there is anything to parse...
		if( xQueueReceive(CMDParser_CMDQueue, &QueueItem, portMAX_DELAY )) {
			//debug_printf("CMD-Parser: Source: %d; Value: %s\n", QueueItem.uxSourceID, QueueItem.ucValue);
			

			//LX200 Command-Parser
			//see http://www.meade.com/support/LX200CommandSet.pdf for more details
            //strncpy(Response.ucValue, 0x15, OD_CMDLENGTH); //unknown command
			sprintf(Response.ucValue, "%c", 0x15);

			if(QueueItem.ucValue[0] == 'o' && QueueItem.ucValue[1] == 'D') {
				//Special openDrive command

				if(QueueItem.ucValue[2] == 'C' && QueueItem.ucValue[3] == 'R') {
					//Set current of RA-Motor
					CMDParser_CMD_oDCR(&QueueItem, &Response);

				} 
			}


			
			//Send response
			switch(QueueItem.uxSourceID) {
				case OD_SID_UART2:		xUART2_SendToQueue(&Response); break;
			}

			
		}
	}
}

portBASE_TYPE xCMDParser_AddCMD2Queue(CMDParser_CMDQueue_t *QueueItem) {

	portBASE_TYPE xStatus = pdFALSE;
    xStatus = xQueueSendToBack(CMDParser_CMDQueue, QueueItem, 0 );

	return(xStatus);
}

void CMDParser_CMD_oDCR(CMDParser_CMDQueue_t *QueueItem, xUART2_SendQueue_t *Response) {
	if(strlen((QueueItem->ucValue)+4)>0) {
		int32_t current = 0;
		current = strtol( ((QueueItem->ucValue)+4), (void*)0, 10 );
		
		if(current == LONG_MAX || current ==  LONG_MIN) {			
			strncpy(Response->ucValue, "ERR_OOR\n", OD_CMDLENGTH); // outside int32_t range!
			return;
		}

        TMC428_SetMotorCurrent(TMC428_MotorRA, &current);
		sprintf(Response->ucValue, "oDCR%d\n", current);		
	} else {		
		strncpy(Response->ucValue, "ERR_NC\n", OD_CMDLENGTH); // no current value send
	}
}