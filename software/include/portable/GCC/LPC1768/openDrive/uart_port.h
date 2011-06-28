#ifndef __openDrive_UART_PORT_H 
#define __openDrive_UART_PORT_H

#include "CMSIS/CM3/DeviceSupport/LPC17xx/LPC17xx.h"
#include "LPC1768_bitdef.h"

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80


/* Interrupt-Handler */
void UART2_IRQHandler(void);
void vUART2Port_ISR_LineStatus(void);
void vUART2Port_ISR_TimeOut(void);
void vUART2Port_ISR_SendChar(void);
void vUART2Port_ISR_ReceiveChar(void);

extern uint32_t SystemCoreClock;

/* 
 * --------------------- UART 1 -------------------
 */
static inline uint32_t xUART1Port_Init(void) {
	assert(0);
}

static inline void vUART1Port_SendChar(unsigned char SendChar) {
  	assert(0);
}

static inline  unsigned char vUART1Port_ReceiveChar(void) {
	assert(0);
}

/* 
 * --------------------- UART 2 -------------------
 */
volatile uint8_t UART2_ReceiveBuffer[UART2_RECEIVE_BUFFER_SIZE];
volatile uint32_t UART2_ReceiveBufferPosW;
volatile uint32_t UART2_Status;
volatile uint8_t UART2_TxEmpty;
uint32_t UART2_ReceiveBufferPosR;


static inline uint32_t xUART2Port_Init(void) {
	// Enable RxD2 P0.11, TxD2 P0.10
	LPC_PINCON->PINSEL0 &= ~(0x03<<20);                     // Reset P0.10 = GPIO   
	LPC_PINCON->PINSEL0 |=  (0x01<<20);                        // Config P0.10 = TxD2
	LPC_PINCON->PINSEL0 &= ~(0x03<<22);                     // Reset P0.11 = GPIO   
	LPC_PINCON->PINSEL0 |=  (0x01<<22);                        // Config P0.11 = RxD2
	
	LPC_SC->PCONP |= (1 << LPC_SC_PCONP_PCUART2);                                   // UART 2 power/clock control bit.
	LPC_SC->PCLKSEL1 |= (0b01 << LPC_SC_PCLKSEL1_PCLK_UART2);          // Prescaler CCLK/1     
	
	// 8 bits, no Parity, 1 Stop bit, DLAB = 1      
	LPC_UART2->LCR  = (0b11 << LPC_UARTn_LCR_WordLengthSelect)
		| (1 << LPC_UARTn_LCR_DivisorLatchAccess);
	
	LPC_UART2->DLM = UART2_DLM;                                                        
	LPC_UART2->DLL = UART2_DLL;
	
	// DLAB must be 0 befor IRQ can be enabled
	LPC_UART2->LCR &= ~(1 << LPC_UARTn_LCR_DivisorLatchAccess);
	
	//Fractional Divider
	LPC_UART2->FDR  = (UART2_DIVADDVAL << LPC_UARTn_FDR_DIVADDVAL)
		| (UART2_MULVAL << LPC_UARTn_FDR_MULVAL);
	
	// Enable and reset TX and RX FIFO.
	LPC_UART2->FCR     = (1 << LPC_UARTn_FCR_FifoEnable)
		| (1 << LPC_UARTn_FCR_RxFifoReset)
		| (1 << LPC_UARTn_FCR_TxFifoReset);
	
	// Enable UART2 interrupt
	NVIC_EnableIRQ(UART2_IRQn);
	LPC_UART2->IER = (1 << LPC_UARTn_IER_RBRInterruptEnable)                        // Enables the Receive Data Available interrupt for UARTn.
		| (1 << LPC_UARTn_IER_THREInterruptEnable)                   // Enables the THRE interrupt for UARTn.
		| (1 << LPC_UARTn_IER_RXLSInterruptEnable);                  // Enables the UARTn RX line status interrupts.
	
	NVIC_SetPriority(UART2_IRQn, 0);
	return 0;
}

static inline void vUART2Port_SendChar(unsigned char SendChar) {
	/* THRE status, contain valid data */
	while( !(UART2_TxEmpty & 0x01) );	

	LPC_UART2->THR = SendChar;
	UART2_TxEmpty = 0;	// not empty in the THR until it shifts out
}

static inline  unsigned char vUART2Port_ReceiveChar(void) {
	unsigned char rcvd;
	rcvd = UART2_ReceiveBuffer[UART2_ReceiveBufferPosR];
	
	//set readbuffer to next char
	if(UART2_ReceiveBufferPosR == UART2_RECEIVE_BUFFER_SIZE-1) {
		UART2_ReceiveBufferPosR = 0;
	} else {
		UART2_ReceiveBufferPosR++;
	}
}

#endif /* end __openDrive_UART_PORT_H */
