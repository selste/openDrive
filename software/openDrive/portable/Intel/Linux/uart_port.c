
#include "debug.h"
#include "LPC17xx.h"
#include "LPC1768_bitdef.h"
#include "types.h"
#include "opendrive-config.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "uart.h"
#include "uart_port.h"

//*******************************************************************************
//*** Interrupt-Handler                                                       ***
//*******************************************************************************

/*** UART2 IRQ-Handler ***/
void UART2_IRQHandler(void) {

	uint8_t IIRValue = LPC_UART2->IIR;
    
	IIRValue >>= 1;			// skip pending bit in IIR
	IIRValue &= 0x07;		// check bit 1~3, interrupt identification

	// Receive Line Status interrupt (RLS)
	if ( IIRValue == 0b011 ) {
		vUART2Port_ISR_LineStatus();
	}
  
	// Receive Data Available interrupt (RDA)
	else if ( IIRValue == 0b010 ) {		
		vUART2Port_ISR_ReceiveChar();
	}

	// Character Time-out Indicator interrupt (CTI)
	else if ( IIRValue == 0b110 ) {
		vUART2Port_ISR_TimeOut();
	}

	// Transmitter Holding Register Empty interrupt (THRE)
	else if ( IIRValue == 0b001 ) {		        
		vUART2Port_ISR_SendChar();
	}

	// Modem Interrupt 
	else {
		debug_printf("Modem Interrupt\n");
	}
}

/*** UART2 Receive Line Status interrupt (RLS) ***/
void vUART2Port_ISR_LineStatus(void) {
  uint8_t LSRValue = LPC_UART2->LSR;
  /* Receive Line Status */
		if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) ) {
			/* There are errors or break interrupt */
			/* Read LSR will clear the interrupt */	

			if(LSRValue & LSR_OE) {debug_printf("Error: Overrun Error\n");}
            if(LSRValue & LSR_PE) {debug_printf("Error: Parity Error\n");}
            if(LSRValue & LSR_FE) {debug_printf("Error: Framing Error\n");}
            if(LSRValue & LSR_RXFE) {debug_printf("Error: Error in RX FIFO\n");}
            if(LSRValue & LSR_BI) {debug_printf("Error: Break Interrupt\n");}
		
			UART2_Status = LSRValue;
			uint8_t Dummy = LPC_UART2->RBR;		/* Dummy read on RX to clear interrupt, then bail out */

			//debug_printf("Dumme: %c\n", Dummy);

			return;
		}
	
		// Receive Data Ready
		if ( LSRValue & LSR_RDR ) {		
			/* If no error on RLS, normal ready, save into the data buffer. */
			/* Note: read RBR will clear the interrupt */
			
            vUART2Port_ISR_ReceiveChar();
		}
}


/*** UART2 Character Time-out Indicator interrupt (CTI) ***/
void vUART2Port_ISR_TimeOut(void) {
	debug_printf("Character timeout indicator\n");
	/* Character Time-out indicator */
	UART2_Status |= 0x100;		/* Bit 9 as the CTI error */
}


/*** UART2 Transmitter Holding Register Empty interrupt (THRE) ***/
void vUART2Port_ISR_SendChar(void) {
	uint8_t LSRValue;
	LSRValue = LPC_UART2->LSR;		// Check status in the LSR to see if valid data in U0THR or not
	if ( LSRValue & LSR_THRE ) {
		UART2_TxEmpty = 1;
	} else {
		UART2_TxEmpty = 0;
	}
}


/*** UART2 Receive Data Available interrupt (RDA) ***/
void vUART2Port_ISR_ReceiveChar(void) {
	static portBASE_TYPE xHigherPriorityTaskWoken;
	unsigned char ReceivedChar = LPC_UART2->RBR; //Read received Char from register

	UART2_ReceiveBuffer[UART2_ReceiveBufferPosW] = ReceivedChar;
	
	if(!UART2_ReceiverTaskRunning) {
		return;
	}

	if(UART2_ReceiveBufferPosW == UART2_RECEIVE_BUFFER_SIZE-1) {
		UART2_ReceiveBufferPosW = 0;
	} else {
		UART2_ReceiveBufferPosW++;
	}

    xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR( UART2_ReceiveCntQueue, &xHigherPriorityTaskWoken );

    if( xHigherPriorityTaskWoken == pdTRUE ) {
		vPortYieldFromISR();
	}    

}


