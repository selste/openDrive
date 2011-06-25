#include "debug.h"

#include "LPC17xx.h"
#include "LPC1768_bitdef.h"
#include "types.h"
#include "opendrive-config.h"

#include "FreeRTOS.h"
#include "task.h"

//#include "led.h"
#include "test.h"
#include "uart.h"
#include "spi.h"
#include "pwm_port.h"
#include "cmdparser.h"
#include "main.h"


int main (void) {

	SystemInit();
	prvSetupHardware();
	
	vUART_Init();
    
	vPWMPort_Init();
    vPWMPort_SetMR(0);
    vPWMPort_StartPWM();


	//vStartTestTask(mainTEST_TASK_PRIORITY);
	//vStartLEDTask(mainLED_TASK_PRIORITY);
	vUART2_StartSenderTask(mainUARTSEND_TASK_PRIORITY);
	vUART2_StartReceiverTask(mainUARTRECEIVE_TASK_PRIORITY);
    //vSPI_StartTask(mainSPI_TASK_PRIORITY);

    vCMDParser_StartTask(mainCMDPARSER_TASK_PRIORITY);
    



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

	//  Setup the peripheral bus to be the same as the PLL output.
	LPC_SC->PCLKSEL0 = 0x05555555;


	// Turn-OFF all LED ..
	//LPC_PINCON->PINSEL4 &= ~(0xFFFF);
    //LPC_GPIO2->FIODIR |= 0xFF;
    //LPC_GPIO2->FIOCLR = 0xFF; 

}


void HardFault_Handler(void) {
	
	//uint32_t test = HFSR;

	while(1);
}