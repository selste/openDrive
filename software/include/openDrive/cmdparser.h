#ifndef __openDrive_CMDPARSER_H 
#define __openDrive_CMDPARSER_H


#define CMDParser_STACK_SIZE		(configMINIMAL_STACK_SIZE*2)
#define CMDParser_CMDQUEUE_ITEMS 10


void vCMDParser_StartTask( unsigned portBASE_TYPE uxPriority );
void vCMDParser_Task( void * pvParameters );
//portBASE_TYPE xCMDParser_AddCMD2Queue(uint32_t CMDID, unsigned char *Value[128], unsigned char *Source[20]);
portBASE_TYPE xCMDParser_AddCMD2Queue(CMDParser_CMDQueue_t *QueueItem);


void CMDParser_CMD_oDCR(CMDParser_CMDQueue_t *QueueItem, xUART2_SendQueue_t *Response);


#endif /* end __openDrive_CMDPARSER_H */