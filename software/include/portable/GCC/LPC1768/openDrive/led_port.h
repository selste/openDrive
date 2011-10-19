#ifndef __openDrive_LED_PORT_H
#define __openDrive_LED_PORT_H

#include "CMSIS/CM3/DeviceSupport/LPC17xx/LPC17xx.h"
#include "LPC1768_bitdef.h"

#define LED_STACK_SIZE		configMINIMAL_STACK_SIZE


/*** Functions ***/
static inline void vLEDPort_Init(void) {
	LPC_SC->PCONP |= 0x02UL;
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3<<2))) | (0x01 << 2);

	LPC_GPIO2->FIODIR = 0x000000FF;				// P2[0..7] defined as Outputs 
	LPC_GPIO2->FIOCLR = 0x000000FF;				// turn off all the LEDs 

}

static inline void vLEDPort_Toggle(uint32_t *counter) {
	LPC_GPIO2->FIOSET = 1 << *counter;
	(*counter)++;

	if ( *counter > 8 ) {
		*counter = 0;
		LPC_GPIO2->FIOCLR = 0x000000FF;
	}
}


#endif /* end __openDrive_LED_PORT_H */
