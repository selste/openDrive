
#ifndef __openDrive_LED_PORT_H
#define __openDrive_LED_PORT_H

#include "stdint.h"

#define LED_STACK_SIZE		configMINIMAL_STACK_SIZE


/*** Functions ***/
void vLEDPort_Init(void);
void vLEDPort_Toggle(uint32_t *counter);


#endif /* end __openDrive_LED_PORT_H */
