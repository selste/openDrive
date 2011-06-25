#ifndef __openDrive_SPI_H 
#define __openDrive_SPI_H

#define SPI_STACK_SIZE		configMINIMAL_STACK_SIZE


void vSPI_StartTask( unsigned portBASE_TYPE uxPriority );
void vSPI_Task( void * pvParameters );

#endif /* end __openDrive_SPI_H */