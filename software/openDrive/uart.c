#include <string.h>

#include "debug.h"
#include "types.h"
#include "opendrive-config.h"

#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/task.h"
#include "FreeRTOS/queue.h"
#include "FreeRTOS/semphr.h"

#include "uart.h"
#include "uart_port.h"
#include "cmdparser.h"


extern uint32_t SystemCoreClock;

uint8_t UART1_IsInitialized, UART1_SenderTaskRunning, UART1_ReceiverTaskRunning;
uint8_t UART2_IsInitialized, UART2_SenderTaskRunning, UART2_ReceiverTaskRunning;
xQueueHandle UART1_SendQueue;
xQueueHandle UART2_SendQueue;
uint8_t UART1_RX;
uint8_t UART2_RX;
volatile xSemaphoreHandle UART1_ReceiveCntQueue;
volatile xSemaphoreHandle UART2_ReceiveCntQueue;

static inline uint32_t xUART1_Init(void) {
	int rc;

	UART2_SendQueue = xQueueCreate(UART2_SENDQUEUE_ITEMS, sizeof(xUART2_SendQueue_t));      // create UART2 SendQueue
	UART2_ReceiveCntQueue = xSemaphoreCreateCounting( UART2_RECEIVE_BUFFER_SIZE, 0 );  // create UART2 Receive CountingSemaphore

	rc = xUART1Port_Init();

	UART1_IsInitialized = 1;
	return rc;
}

static inline uint32_t xUART2_Init(void) {
	int rc;

	UART2_SendQueue = xQueueCreate(UART2_SENDQUEUE_ITEMS, sizeof(xUART2_SendQueue_t));      // create UART2 SendQueue
	UART2_ReceiveCntQueue = xSemaphoreCreateCounting( UART2_RECEIVE_BUFFER_SIZE, 0 );  // create UART2 Receive CountingSemaphore
	
	rc = xUART2Port_Init();
	
	UART2_IsInitialized = 1;
	return rc;
}

void vUART_Init(void) {
	xUART1_Init();
	xUART2_Init();
}

static inline void vUART2_SendChar(unsigned char SendChar) {
  vUART2Port_SendChar(SendChar);
}
static inline void vUART1_SendChar(unsigned char SendChar) {
  vUART1Port_SendChar(SendChar);
}


static inline void vUART2_SendCString(unsigned char *BufferPtr) {
	while(*BufferPtr) {
		vUART2_SendChar(*BufferPtr);
		BufferPtr++;
	}
	return;
}

static inline void vUART1_SendCString(unsigned char *BufferPtr) {
	while(*BufferPtr) {
		vUART1_SendChar(*BufferPtr);
		BufferPtr++;
	}
	return;
}

static inline unsigned char vUART1_ReceiveChar() {
  return vUART1Port_ReceiveChar();
}
static inline unsigned char vUART2_ReceiveChar() {
  return vUART2Port_ReceiveChar();
}

void vUART2_StartSenderTask( unsigned portBASE_TYPE uxPriority ) {
	static unsigned char ucParameterToPass; 
	xTaskHandle xHandle;
	
	if(UART2_IsInitialized) {
		portBASE_TYPE xStatus = xTaskCreate( vUART2_SenderTask, (signed portCHAR *) "USEND", 
						     UART_STACK_SIZE, &ucParameterToPass, uxPriority, &xHandle );	
        
		if(xStatus == pdTRUE) {
			UART2_SenderTaskRunning = 1;
		} else {
			debug_printf("vUART2_StartReceiverTask: Error - Can not start SenderTask!\n");
		}
	} else {
		debug_printf("vUART2_StartSenderTask: Error - UART2 not initialized!\n");
	}

	return;
} 

void vUART2_StartReceiverTask( unsigned portBASE_TYPE uxPriority ) {
  static unsigned char ucParameterToPass; 
	xTaskHandle xHandle;
    
	if(UART2_IsInitialized) {
		portBASE_TYPE xStatus = xTaskCreate( vUART2_ReceiverTask, (signed portCHAR *) "URec", UART_STACK_SIZE, &ucParameterToPass, uxPriority, &xHandle );	

        if(xStatus == pdTRUE) {
			UART2_ReceiverTaskRunning = 1;
		} else {
			debug_printf("vUART2_StartReceiverTask: Error - Can not start ReceiverTask!\n");
		}
	} else {
		debug_printf("vUART2_StartReceiverTask: Error - UART2 not initialized!\n");
	}

	return;
} 

void vUART2_SenderTask( void * pvParameters ) {
	( void ) pvParameters;
	xUART2_SendQueue_t QueueItem;

	vUART2_SendCString("UART 2 Output started\n");
	while(1) {
		//Sleep until there is anything to send...
		if( xQueueReceive(UART2_SendQueue, &QueueItem, portMAX_DELAY )) {

			vUART2_SendCString( QueueItem.ucValue );
			debug_printf("UART2_SEND '%d': %s\n", QueueItem.uxSourceID, QueueItem.ucValue);

		}
	}
}

void vUART2_ReceiverTask( void * pvParameters ) {
	( void ) pvParameters;
	unsigned char ReceivedChar;
	
	static unsigned char StringBuffer[OD_CMDLENGTH];
	static uint32_t UART2_Count = 0;	

	while(1) {
		//Sleep until there is anything received...
		if( xSemaphoreTake( UART2_ReceiveCntQueue, portMAX_DELAY ) == pdTRUE ) {
			
			//read char from buffer
			ReceivedChar = vUART2Port_ReceiveChar();
			
			//interpret received char 
			if(ReceivedChar == UART2_StringStartChar) {
				//begin receiving a new string...
				UART2_RX = 1;
				UART2_Count = 0;
			}
			
			else if(ReceivedChar == UART2_StringEndChar) {
				if(UART2_RX) {	
					//received complete string...
					StringBuffer[UART2_Count] = (unsigned char)'\0';					
					UART2_RX = 0;
					UART2_Count = 0;
					
					CMDParser_CMDQueue_t Item;
					memcpy(Item.ucValue, StringBuffer, OD_CMDLENGTH);
					Item.uxSourceID = OD_SID_UART2;
					
					xCMDParser_AddCMD2Queue(&Item);
				} else {
					//received incomplete string...
					UART2_RX = 0;
					UART2_Count = 0;
					debug_printf("NACK\n");                
				}
			}
			else {
				//adding char to string...
				if(!UART2_RX) {
					debug_printf("Char '%c' dropped...\n", ReceivedChar);
					continue;
				}
				
				StringBuffer[UART2_Count] = ReceivedChar;
				UART2_Count++;
			}
		}
	}
}

portBASE_TYPE xUART2_SendToQueue(xUART2_SendQueue_t *Item) {

	portBASE_TYPE xStatus = pdFALSE;
	
	//extern xQueueHandle UART2_SendQueue;
	if(!UART2_IsInitialized ) {
		debug_printf("xUART2_SendToQueue: Error - UART2 not initialized!\n");
	}
	
	else if(!UART2_SenderTaskRunning) {
		debug_printf("xUART2_SendToQueue: Error - UART2 SenderTask not running!\n");
	}

	else {
		xStatus = xQueueSendToBack(UART2_SendQueue, Item, 0 ); 	
	}

	return(xStatus);
}
