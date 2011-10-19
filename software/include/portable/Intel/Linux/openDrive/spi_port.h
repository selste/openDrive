#ifndef __openDrive_SPI_PORT_H 
#define __openDrive_SPI_PORT_H



/*** SPI CS Multiplexing
 * P2.0 -> CS; P2.1 -> CS_A; P2.2 -> CS_B; P2.3 -> CS_C
 * P2.0 must be low to use mutliplexer
 * P2.1/2/3 used to select port
 * See 74HC138 Datasheet for more informations
 ***/

#define SPICS_FIRSTBIT	0
#define SPICS_ALLHIGH	LPC_GPIO2->FIOSET = (0b1111 << SPICS_FIRSTBIT);
#define SPICS_Y0LOW		SPICS_ALLHIGH; LPC_GPIO2->FIOCLR = (~(0b0000) << SPICS_FIRSTBIT);
#define SPICS_Y1LOW		SPICS_ALLHIGH; LPC_GPIO2->FIOCLR = (~(0b0010) << SPICS_FIRSTBIT);
#define SPICS_Y2LOW		SPICS_ALLHIGH; LPC_GPIO2->FIOCLR = (~(0b0100) << SPICS_FIRSTBIT);
#define SPICS_Y3LOW		SPICS_ALLHIGH; LPC_GPIO2->FIOCLR = (~(0b0110) << SPICS_FIRSTBIT);
#define SPICS_Y4LOW		SPICS_ALLHIGH; LPC_GPIO2->FIOCLR = (~(0b1000) << SPICS_FIRSTBIT);
#define SPICS_Y5LOW		SPICS_ALLHIGH; LPC_GPIO2->FIOCLR = (~(0b1010) << SPICS_FIRSTBIT);
#define SPICS_Y6LOW		SPICS_ALLHIGH; LPC_GPIO2->FIOCLR = (~(0b1100) << SPICS_FIRSTBIT);
#define SPICS_Y7LOW		SPICS_ALLHIGH; LPC_GPIO2->FIOCLR = (~(0b1110) << SPICS_FIRSTBIT);


#define TMC428_SPI_RA_Active	SPICS_Y0LOW
#define TMC428_SPI_RA_Inactive	SPICS_ALLHIGH


//SPI Status Register
#define SPI_ABRT 0b00000100
#define SPI_MODF 0b00001000
#define SPI_ROVR 0b00010000
#define SPI_WCOL 0b00100000
#define SPI_SPIF 0x80

//SSPn Status Register
#define SSPN_TFE 0b00001		// Transmit FIFO Empty
#define SSPN_TNF 0b00010		// Transmit FIFO Not Full
#define SSPN_RNE 0b00100		// Receive FIFO Not Empty
#define SSPN_RFF 0b01000		// Receive FIFO Full
#define SSPN_BSY 0b10000		// Busy

#define SPI_SSEL_START
#define SPI_SSEL_STOP

#define SSP0_SSEL_HIGH		LPC_GPIO0->FIOSET = (1 << 16)
#define SSP0_SSEL_LOW		LPC_GPIO0->FIOCLR = (1 << 16)

#define SSP1_SSEL_HIGH		LPC_GPIO0->FIOSET = (1 << 6)
#define SSP1_SSEL_LOW		LPC_GPIO0->FIOCLR = (1 << 6)



uint32_t xSSP0Port_Init(void);
unsigned char xSSP0Port_Transmit(unsigned char SendChar);
unsigned char xSSP0Port_Receive(unsigned char *ReceivedChar);
void xSSP0Port_SendDummyFrame(uint32_t cnt);
void xSSP0Port_ClearRxFifo(void) ;

uint32_t xSSP1Port_Init(void);
unsigned char xSSP1Port_Transmit(unsigned char SendChar);
unsigned char xSSP1Port_Receive(unsigned char *ReceivedChar);
void xSSP1Port_SendDummyFrame(uint32_t cnt);
void xSSP_CSMP_Init(void);


#endif /* end __openDrive_SPI_PORT_H */