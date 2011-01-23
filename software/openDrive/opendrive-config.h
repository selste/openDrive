#ifndef __openDrive_CONFIG_H 
#define __openDrive_CONFIG_H



#define OD_CMDLENGTH	64



// Task priorities.
#define mainLED_TASK_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainTEST_TASK_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainUARTSEND_TASK_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define mainUARTRECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define mainSPI_TASK_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCMDPARSER_TASK_PRIORITY			( tskIDLE_PRIORITY + 2 )

// Source-IDs (uint32_t)
// Used to identify the Source of an command
#define OD_SID_CMDPARSER 1
#define OD_SID_UART0 2
#define OD_SID_UART1 3
#define OD_SID_UART2 4
#define OD_SID_SPI 5
#define OD_SID_SSP0 6
#define OD_SID_SSP1 7
#define OD_SID_TESTTASK 8



#endif /* end __openDrive_CONFIG_H */
