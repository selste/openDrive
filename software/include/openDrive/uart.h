#ifndef __openDrive_UART_H 
#define __openDrive_UART_H

#define UART_STACK_SIZE		configMINIMAL_STACK_SIZE


/*
#define UART2_BAUDRATE 1000000
#define UART2_DIVADDVAL 1
#define UART2_MULVAL 4
#define UART2_DLM 0
#define UART2_DLL 5
*/


#define UART2_BAUDRATE 1504629
#define UART2_DIVADDVAL 5
#define UART2_MULVAL 13
#define UART2_DLM 0
#define UART2_DLL 3


#define UART2_RECEIVE_BUFFER_SIZE 30
#define UART2_SENDQUEUE_ITEMS 10

#define UART2_StringStartChar (unsigned char) ':'
#define UART2_StringEndChar (unsigned char) '#'
#define UART2_ReceiveInvalid 'NACK\0'


/* FreeRTOS Functions */
void vUART_Init(void);
void vUART2_StartSenderTask( unsigned portBASE_TYPE uxPriority );
void vUART2_StartReceiverTask( unsigned portBASE_TYPE uxPriority );
void vUART2_SenderTask( void * pvParameters );
void vUART2_ReceiverTask( void * pvParameters );
portBASE_TYPE xUART2_SendToQueue(xUART2_SendQueue_t *Item);

#endif /* end __openDrive_UART_H */
