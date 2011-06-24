
#ifndef __openDrive_LED_H
#define __openDrive_LED_H

#define LED_STACK_SIZE		configMINIMAL_STACK_SIZE


/*** Functions ***/
void vStartLEDTask( unsigned portBASE_TYPE uxPriority );
void vLEDTask( void * pvParameters );


#endif /* end __openDrive_LED_H */
