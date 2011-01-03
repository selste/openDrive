
#include "LPC17xx.h"
#include "LPC1768_bitdef.h"

#include "FreeRTOS.h"
#include "task.h"

#include "led.h"
#include "main.h"


int main (void) {

	SystemInit();
	prvSetupHardware();


	vStartLEDTask(mainLED_TASK_PRIORITY);

	vTaskStartScheduler();

	for( ;; );

	return(0);
}

void prvSetupHardware( void ) {

	// Disable peripherals power.
	LPC_SC->PCONP = 0;

	// Enable GPIO power.
	LPC_SC->PCONP = (1 << LPC_SC_PCONP_PCGPIO);

	// Disable TPIU.
	LPC_PINCON->PINSEL10 = 0;

	//  Setup the peripheral bus to be the same as the PLL output (64 MHz).
	LPC_SC->PCLKSEL0 = 0x05555555;

}


