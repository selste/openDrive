#ifndef __LPC1768BITDEF_H 
#define __LPC1768BITDEF_H


#define LPC_TIMx_MCR_MR0I 0
#define LPC_TIMx_MCR_MR0R 1
#define LPC_TIMx_MCR_MR0s 2
#define LPC_TIMx_MCR_MR1I 3
#define LPC_TIMx_MCR_MR1R 4
#define LPC_TIMx_MCR_MR1s 5
#define LPC_TIMx_MCR_MR2I 6
#define LPC_TIMx_MCR_MR2R 7
#define LPC_TIMx_MCR_MR2s 8
#define LPC_TIMx_MCR_MR3I 9
#define LPC_TIMx_MCR_MR3R 10
#define LPC_TIMx_MCR_MR3s 11


/**************************************************************
Peripheral Clock Selection registers
LPC_SC->PCLKSEL0 & LPC_SC->PCLKSEL1

Values:
	00 -> PCLK_peripheral = CCLK/4
	01 -> PCLK_peripheral = CCLK
	10 -> PCLK_peripheral = CCLK/2
	11 -> PCLK_peripheral = CCLK/8, except for CAN1, CAN2, and CAN filtering when “11” selects = CCLK/6.
**************************************************************/



#define LPC_SC_PCLKSEL0_PCLK_WDT 0		// Peripheral clock selection for WDT. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_TIMER0 2	// Peripheral clock selection for TIMER0. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_TIMER1 4	// Peripheral clock selection for TIMER1. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_UART0 6	// Peripheral clock selection for UART0. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_UART1 8	// Peripheral clock selection for UART1. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_PWM1 12	// Peripheral clock selection for PWM1. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_I2C0 14	// Peripheral clock selection for I2C0. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_SPI 16		// Peripheral clock selection for SPI. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_SSP1 20	// Peripheral clock selection for SSP1. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_DAC 22		// Peripheral clock selection for DAC. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_ADC 24		// Peripheral clock selection for ADC. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_CAN1 26	// Peripheral clock selection for CAN1. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_CAN2 28	// Peripheral clock selection for CAN2. (Default: 00)
#define LPC_SC_PCLKSEL0_PCLK_ACF 30		// Peripheral clock selection for CAN acceptance filtering. (Default: 00)


#define LPC_SC_PCLKSEL1_PCLK_QEI 0		// Peripheral clock selection for the Quadrature Encoder Interface. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_GPIOINT 2	// Peripheral clock selection for GPIO interrupts. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_PCB 4		// Peripheral clock selection for the Pin Connect block. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_I2C1 6		// Peripheral clock selection for I2C1. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_SSP0 10	// Peripheral clock selection for SSP0. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_TIMER2 12	// Peripheral clock selection for TIMER2. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_TIMER3 14	// Peripheral clock selection for TIMER3. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_UART2 16	// Peripheral clock selection for UART2. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_UART3 18	// Peripheral clock selection for UART3. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_I2C2 20	// Peripheral clock selection for I2C2. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_I2S 22		// Peripheral clock selection for I2S. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_RIT 26		// Peripheral clock selection for Repetitive Interrupt Timer. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_SYSCON 28	// Peripheral clock selection for the System Control block. (Default: 00)
#define LPC_SC_PCLKSEL1_PCLK_MC 30		// Peripheral clock selection for the Motor Control PWM. (Default: 00)



/**************************************************************
Power Control for Peripherals register
LPC_SC->PCONP

Values:
	0 -> Off
	1 -> On
**************************************************************/

#define LPC_SC_PCONP_PCTIM0 1			// Timer/Counter 0 power/clock control bit. (Default: 1)
#define LPC_SC_PCONP_PCTIM1 2			// Timer/Counter 1 power/clock control bit. (Default: 1)
#define LPC_SC_PCONP_PCUART0 3			// UART0 power/clock control bit. (Default: 1)
#define LPC_SC_PCONP_PCUART1 4			// UART1 power/clock control bit. (Default: 1)
#define LPC_SC_PCONP_PCPWM1 6			// PWM1 power/clock control bit. (Default: 1)
#define LPC_SC_PCONP_PCI2C0 7			// The I2C0 interface power/clock control bit. (Default: 1)
#define LPC_SC_PCONP_PCSPI 8			// The SPI interface power/clock control bit. (Default: 1)
#define LPC_SC_PCONP_PCRTC 9			// The RTC power/clock control bit. (Default: 1)
#define LPC_SC_PCONP_PCSSP1 10			// The SSP 1 interface power/clock control bit. (Default: 1)
#define LPC_SC_PCONP_PCADC 12			// A/D converter (ADC) power/clock control bit. Note: Clear the PDN bit in the AD0CR before clearing this bit, and set this bit before setting PDN. (Default: 0)
#define LPC_SC_PCONP_PCCAN1 13			// CAN Controller 1 power/clock control bit. (Default: 0)
#define LPC_SC_PCONP_PCCAN2 14			// CAN Controller 2 power/clock control bit. (Default: 0)
#define LPC_SC_PCONP_PCGPIO 15			// Power/clock control bit for IOCON, GPIO, and GPIO interrupts. (Default: 1)
#define LPC_SC_PCONP_PCRIT 16			// Repetitive Interrupt Timer power/clock control bit. (Default: 0)
#define LPC_SC_PCONP_PCMCPWM 17			// Motor Control PWM (Default: 0)
#define LPC_SC_PCONP_PCQEI 18			// Quadrature Encoder Interface power/clock control bit. (Default: 0)
#define LPC_SC_PCONP_PCI2C1 19			// The I2C1 interface power/clock control bit. (Default: 1)
#define LPC_SC_PCONP_PCSSP0 21			// The SSP0 interface power/clock control bit. (Default: 1)
#define LPC_SC_PCONP_PCTIM2 22			// Timer 2 power/clock control bit. (Default: 0)
#define LPC_SC_PCONP_PCTIM3 23			// Timer 3 power/clock control bit. (Default: 0)
#define LPC_SC_PCONP_PCUART2 24			// UART 2 power/clock control bit. (Default: 0)
#define LPC_SC_PCONP_PCUART3 25			// UART 3 power/clock control bit. (Default: 0)
#define LPC_SC_PCONP_PCI2C2 26			// I2C interface 2 power/clock control bit. (Default: 1)
#define LPC_SC_PCONP_PCI2S 27			// I2S interface power/clock control bit. (Default: 0)
#define LPC_SC_PCONP_PCGPDMA 29			// GPDMA function power/clock control bit. (Default: 0)
#define LPC_SC_PCONP_PCENET 30			// Ethernet block power/clock control bit. (Default: 0)
#define LPC_SC_PCONP_PCUSB 31			// USB interface power/clock control bit. (Default: 0)


/**************************************************************
UARTn Line Control Register
LPC_UARTn->LCR
**************************************************************/

#define LPC_UARTn_LCR_WordLengthSelect 0		// Word Length Select (Default: 00)
#define LPC_UARTn_LCR_StopBitSelect 2			// Stop Bit Select (Default: 0)
#define LPC_UARTn_LCR_ParityEnable 3			// Parity Enable (Default: 0)
#define LPC_UARTn_LCR_ParitySelect 4			// Parity Select (Default: 00)
#define LPC_UARTn_LCR_BreakControl 6			// Break Control (Default: 0)
#define LPC_UARTn_LCR_DivisorLatchAccess 7		// Divisor Latch Access Bit (DLAB) (Default: 0)


/**************************************************************
UARTn FIFO Control Register
LPC_UARTn->FCR
**************************************************************/

#define LPC_UARTn_FCR_FifoEnable 0			// FIFO Enable (Default: 0)
#define LPC_UARTn_FCR_RxFifoReset 1			// RX FIFO (Default: 0)
#define LPC_UARTn_FCR_TxFifoReset 2			// TX FIFO (Default: 0)
#define LPC_UARTn_FCR_DmaModeSelect 3		// DMA Mode Select (Default: 0)
#define LPC_UARTn_FCR_RxTriggerLevel 6		// RX Trigger Level (Default: 00)


/**************************************************************
UARTn Interrupt Enable Register
LPC_UARTn->IER
**************************************************************/

#define LPC_UARTn_IER_RBRInterruptEnable 0		// RBR Interrupt Enable (Default: 0)
#define LPC_UARTn_IER_THREInterruptEnable 1		// THRE Interrupt Enable (Default: 0)
#define LPC_UARTn_IER_RXLSInterruptEnable 2		// RX Line Status Interrupt Enable (Default: 0)
#define LPC_UARTn_IER_ABEOIntEn 8				// ABEOIntEn (Default: 0)
#define LPC_UARTn_IER_ABTOIntEn 9				// ABTOIntEn (Default: 0)


/**************************************************************
UARTn Fractional Divider Register
LPC_UARTn->FDR

Values:
	1 <= MULVAL <= 15
	0 <= DIVADDVAL <= 14
	DIVADDVAL < MULVAL
**************************************************************/

#define LPC_UARTn_FDR_DIVADDVAL 0				// Baud-rate generation pre-scaler divisor value. (Default: 0)
#define LPC_UARTn_FDR_MULVAL 4					// Baud-rate pre-scaler multiplier value. (Default: 1)


/**************************************************************
UARTn Interrupt Identification Register
LPC_UARTn->IIR
**************************************************************/

#define LPC_UARTn_IIR_IntStatus 0				// Interrupt status. (Default: 1)
#define LPC_UARTn_IIR_IntId 1					// Interrupt identification. (Default: 0)
#define LPC_UARTn_IIR_FIFOEnable 6				// Copies of UnFCR (Default: 0)
#define LPC_UARTn_IIR_ABEOInt 8					// End of auto-baud interrupt. (Default: 0)
#define LPC_UARTn_IIR_ABTOInt 9					// Auto-baud time-out interrupt. (Default: 0)


/**************************************************************
UARTn Line Status Register
LPC_UARTn->LSR
**************************************************************/

#define LPC_UARTn_LSR_RDR 0						// Receiver Data Ready (Default: 0)
#define LPC_UARTn_LSR_OE 1						// Overrun Error (Default: 0)
#define LPC_UARTn_LSR_PE 2						// Parity Error (Default: 0)
#define LPC_UARTn_LSR_FE 3						// Framing Error (Default: 0)
#define LPC_UARTn_LSR_BI 4						// Break Interrupt (Default: 0)
#define LPC_UARTn_LSR_THRE 5					// Transmitter Holding Register Empty (Default: 1)
#define LPC_UARTn_LSR_TEMT 6					// Transmitter Empty (Default: 1)
#define LPC_UARTn_LSR_RXFE 7					// Error in RX FIFO (Default: 0)





#endif