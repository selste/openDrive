#include "debug.h"

// #include "CMSIS/CM3/DeviceSupport/LPC17xx/LPC17xx.h"
// #include "LPC1768_bitdef.h"
#include "types.h"
#include "opendrive-config.h"
#include "portable.h"

#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/task.h"

//#include "led.h"
#include "test.h"
#include "uart.h"
#include "spi.h"
#include "pwm.h"
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

//	vCMDParser_StartTask(mainCMDPARSER_TASK_PRIORITY);
	vStartLEDTask( 10 ) ;    
	vTaskStartScheduler();

	for( ;; ) sleep(10);

	return(0);
}



void HardFault_Handler(void) {
	
	//uint32_t test = HFSR;

	while(1);
}
