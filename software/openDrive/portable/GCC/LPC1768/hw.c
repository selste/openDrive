#include "portable.h"

void prvSetupHardware( void ) {

	// Disable peripherals power.
	LPC_SC->PCONP = 0;

	// Enable GPIO power.
	LPC_SC->PCONP = (1 << LPC_SC_PCONP_PCGPIO);

	// Disable TPIU.
	LPC_PINCON->PINSEL10 = 0;

	//  Setup the peripheral bus to be the same as the PLL output.
	LPC_SC->PCLKSEL0 = 0x05555555;


	// Turn-OFF all LED ..
	//LPC_PINCON->PINSEL4 &= ~(0xFFFF);
    //LPC_GPIO2->FIODIR |= 0xFF;
    //LPC_GPIO2->FIOCLR = 0xFF; 

}
