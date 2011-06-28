#ifndef __LPC1768BITDEF_H 
#define __LPC1768BITDEF_H


#define LPC_PINCON_PINSEL0_P0_0 0			// GPIO Port 0.0; RD1; TXD3; SDA1
#define LPC_PINCON_PINSEL0_P0_1 2			// GPIO Port 0.1; TD1; RXD3; SCL1
#define LPC_PINCON_PINSEL0_P0_2 4			// GPIO Port 0.2; TXD0; AD0.7
#define LPC_PINCON_PINSEL0_P0_3 6			// GPIO Port 0.3; RXD0; AD0.6
#define LPC_PINCON_PINSEL0_P0_4 8			// GPIO Port 0.4; I2SRX_CLK; RD2; CAP2.0
#define LPC_PINCON_PINSEL0_P0_5 10			// GPIO Port 0.5; I2SRX_WS; TD2; CAP2.1
#define LPC_PINCON_PINSEL0_P0_6 12			// GPIO Port 0.6; I2SRX_SDA; SSEL1; MAT2.0
#define LPC_PINCON_PINSEL0_P0_7 14			// GPIO Port 0.7; I2SRX_CLK; SCK1; MAT2.1
#define LPC_PINCON_PINSEL0_P0_8 16			// GPIO Port 0.8; I2SRX_WS; MISO1; MAT2.2
#define LPC_PINCON_PINSEL0_P0_9 18			// GPIO Port 0.9; I2SRX_SDA; MOSI1; MAT2.3
#define LPC_PINCON_PINSEL0_P0_10 20			// GPIO Port 0.10; TXD2; SDA2; MAT3.0
#define LPC_PINCON_PINSEL0_P0_11 22			// PIO Port 0.11; RXD2; SCL2; MAT3.1
#define LPC_PINCON_PINSEL0_P0_15 30			// GPIO Port 0.15; TXD1; SCK0; SCK

#define LPC_PINCON_PINSEL1_P0_16 0			// GPIO Port 0.16; RXD1; SSEL0; SSEL
#define LPC_PINCON_PINSEL1_P0_17 2			// GPIO Port 0.17; CTS1; MISO0; MISO
#define LPC_PINCON_PINSEL1_P0_18 4			// GPIO Port 0.18; DCD1; MOSI0; MOSI
#define LPC_PINCON_PINSEL1_P0_19 6			// GPIO Port 0.19; DSR1; SDA1
#define LPC_PINCON_PINSEL1_P0_20 8			// GPIO Port 0.20; DTR1; SCL1
#define LPC_PINCON_PINSEL1_P0_21 10			// GPIO Port 0.21; RI1; RD1
#define LPC_PINCON_PINSEL1_P0_22 12			// GPIO Port 0.22; RTS1; TD1
#define LPC_PINCON_PINSEL1_P0_23 14			// GPIO Port 0.23; AD0.0; I2SRX_CLK; CAP3.0
#define LPC_PINCON_PINSEL1_P0_24 16			// GPIO Port 0.24; AD0.1; I2SRX_WS; CAP3.1
#define LPC_PINCON_PINSEL1_P0_25 18			// GPIO Port 0.25; AD0.2; I2SRX_SDA; TXD3
#define LPC_PINCON_PINSEL1_P0_26 20			// GPIO Port 0.26; AD0.3; AOUT; RXD3
#define LPC_PINCON_PINSEL1_P0_27 22			// GPIO Port 0.27; SDA0; USB_SDA
#define LPC_PINCON_PINSEL1_P0_28 24			// GPIO Port 0.28; SCL0; USB_SCL

#define LPC_PINCON_PINSEL2_P1_0 0			// GPIO Port 1.0; ENET_TXD0
#define LPC_PINCON_PINSEL2_P1_1 2			// GPIO Port 1.1; ENET_TXD1
#define LPC_PINCON_PINSEL2_P1_4 8			// GPIO Port 1.4; ENET_TX_EN
#define LPC_PINCON_PINSEL2_P1_8 16			// GPIO Port 1.8; ENET_CRS
#define LPC_PINCON_PINSEL2_P1_9 18			// GPIO Port 1.9; ENET_RXD0
#define LPC_PINCON_PINSEL2_P1_10 20			// GPIO Port 1.10; ENET_RXD1
#define LPC_PINCON_PINSEL2_P1_14 28			// GPIO Port 1.14; ENET_RX_ER
#define LPC_PINCON_PINSEL2_P1_15 30			// GPIO Port 1.15; ENET_REF_CLK

#define LPC_PINCON_PINSEL3_P1_16 0			// GPIO Port 1.16; ENET_MDC
#define LPC_PINCON_PINSEL3_P1_17 2			// GPIO Port 1.17; ENET_MDIO
#define LPC_PINCON_PINSEL3_P1_18 4			// GPIO Port 1.18; USB_UP_LED; PWM1.1; CAP1.0
#define LPC_PINCON_PINSEL3_P1_19 6			// GPIO Port 1.19; MCOA0; USB_PPWR; CAP1.1
#define LPC_PINCON_PINSEL3_P1_20 8			// GPIO Port 1.20; MCI0; PWM1.2; SCK0
#define LPC_PINCON_PINSEL3_P1_21 10			// GPIO Port 1.21; MCABORT; PWM1.3; SSEL0
#define LPC_PINCON_PINSEL3_P1_22 12			// GPIO Port 1.22; MCOB0; USB_PWRD; MAT1.0
#define LPC_PINCON_PINSEL3_P1_23 14			// GPIO Port 1.23; MCI1; PWM1.4; MISO0
#define LPC_PINCON_PINSEL3_P1_24 16			// GPIO Port 1.24; MCI2; PWM1.5; MOSI0
#define LPC_PINCON_PINSEL3_P1_25 18			// GPIO Port 1.25; MCOA1; MAT1.1
#define LPC_PINCON_PINSEL3_P1_26 20			// GPIO Port 1.26; MCOB1; PWM1.6; CAP0.0
#define LPC_PINCON_PINSEL3_P1_27 22			// GPIO Port 1.27; CLKOUT; USB_OVRCR; CAP0.1
#define LPC_PINCON_PINSEL3_P1_28 24			// GPIO Port 1.28; MCOA2; PCAP1.0; MAT0.0
#define LPC_PINCON_PINSEL3_P1_29 26			// GPIO Port 1.29; MCOB2; PCAP1.1; MAT0.1
#define LPC_PINCON_PINSEL3_P1_30 28			// GPIO Port 1.30; VBUS; AD0.4
#define LPC_PINCON_PINSEL3_P1_31 30			// GPIO Port 1.31; SCK1; AD0.5

#define LPC_PINCON_PINSEL4_P2_0 0			// GPIO Port 2.0; PWM1.1; TXD1
#define LPC_PINCON_PINSEL4_P2_1 2			// GPIO Port 2.1; PWM1.2; RXD1
#define LPC_PINCON_PINSEL4_P2_2 4			// GPIO Port 2.2; PWM1.3; CTS1
#define LPC_PINCON_PINSEL4_P2_3 6			// GPIO Port 2.3; PWM1.4; DCD1
#define LPC_PINCON_PINSEL4_P2_4 8			// GPIO Port 2.4; PWM1.5; DSR1
#define LPC_PINCON_PINSEL4_P2_5 10			// GPIO Port 2.5; PWM1.6; DTR1
#define LPC_PINCON_PINSEL4_P2_6 12			// GPIO Port 2.6; PCAP1.0; RI1
#define LPC_PINCON_PINSEL4_P2_7 14			// GPIO Port 2.7; RD2; RTS1
#define LPC_PINCON_PINSEL4_P2_8 16			// GPIO Port 2.8; TD2; TXD2; ENET_MDC
#define LPC_PINCON_PINSEL4_P2_9 18			// GPIO Port 2.9; USB_CONNECT; RXD2; ENET_MDIO
#define LPC_PINCON_PINSEL4_P2_10 20			// GPIO Port 2.10; EINT0; NMI
#define LPC_PINCON_PINSEL4_P2_11 22			// GPIO Port 2.11; EINT1; I2STX_CLK
#define LPC_PINCON_PINSEL4_P2_12 24			// GPIO Port 2.12; EINT2; I2STX_WS
#define LPC_PINCON_PINSEL4_P2_13 26			// GPIO Port 2.13; EINT3; I2STX_SDA

#define LPC_PINCON_PINSEL9_P4_28 24			// GPIO Port 4.28; RX_MCLK; MAT2.0; TXD3
#define LPC_PINCON_PINSEL9_P4_29 26			// GPIO Port 4.29; TX_MCLK; MAT2.1; RXD3


#define LPC_PINCON_PINMODE0_P0_0 0			// Port 0 pin 0 control
#define LPC_PINCON_PINMODE0_P0_1 2			// Port 0 pin 1 control
#define LPC_PINCON_PINMODE0_P0_2 4			// Port 0 pin 2 control
#define LPC_PINCON_PINMODE0_P0_3 6			// Port 0 pin 3 control
#define LPC_PINCON_PINMODE0_P0_4 8			// Port 0 pin 4 control
#define LPC_PINCON_PINMODE0_P0_5 10			// Port 0 pin 5 control
#define LPC_PINCON_PINMODE0_P0_6 12			// Port 0 pin 6 control
#define LPC_PINCON_PINMODE0_P0_7 14			// Port 0 pin 7 control
#define LPC_PINCON_PINMODE0_P0_8 16			// Port 0 pin 8 control
#define LPC_PINCON_PINMODE0_P0_9 18			// Port 0 pin 9 control
#define LPC_PINCON_PINMODE0_P0_10 20		// Port 0 pin 10 control
#define LPC_PINCON_PINMODE0_P0_11 22		// Port 0 pin 11 control
#define LPC_PINCON_PINMODE0_P0_15 30		// Port 0 pin 15 control

#define LPC_PINCON_PINMODE1_P0_16 0			// Port 0 pin 16 control
#define LPC_PINCON_PINMODE1_P0_17 2			// Port 0 pin 17 control
#define LPC_PINCON_PINMODE1_P0_18 4			// Port 0 pin 18 control
#define LPC_PINCON_PINMODE1_P0_19 6			// Port 0 pin 19 control
#define LPC_PINCON_PINMODE1_P0_20 8			// Port 0 pin 20 control
#define LPC_PINCON_PINMODE1_P0_21 10		// Port 0 pin 21 control
#define LPC_PINCON_PINMODE1_P0_22 12		// Port 0 pin 22 control
#define LPC_PINCON_PINMODE1_P0_23 14		// Port 0 pin 23 control
#define LPC_PINCON_PINMODE1_P0_24 16		// Port 0 pin 24 control
#define LPC_PINCON_PINMODE1_P0_25 18		// Port 0 pin 25 control
#define LPC_PINCON_PINMODE1_P0_26 20		// Port 0 pin 26 control

#define LPC_PINCON_PINMODE4_P2_0 0			// Port 2 pin 0 control
#define LPC_PINCON_PINMODE4_P2_1 2			// Port 2 pin 1 control
#define LPC_PINCON_PINMODE4_P2_2 4			// Port 2 pin 2 control
#define LPC_PINCON_PINMODE4_P2_3 6			// Port 2 pin 3 control
#define LPC_PINCON_PINMODE4_P2_4 8			// Port 2 pin 4 control
#define LPC_PINCON_PINMODE4_P2_5 10			// Port 2 pin 5 control
#define LPC_PINCON_PINMODE4_P2_6 12			// Port 2 pin 6 control
#define LPC_PINCON_PINMODE4_P2_7 14			// Port 2 pin 7 control
#define LPC_PINCON_PINMODE4_P2_8 16			// Port 2 pin 8 control
#define LPC_PINCON_PINMODE4_P2_9 18			// Port 2 pin 9 control
#define LPC_PINCON_PINMODE4_P2_10 20		// Port 2 pin 10 control
#define LPC_PINCON_PINMODE4_P2_11 22		// Port 2 pin 11 control
#define LPC_PINCON_PINMODE4_P2_12 24		// Port 2 pin 12 control
#define LPC_PINCON_PINMODE4_P2_13 26		// Port 2 pin 13 control


#define LPC_PINCON_PINMODE9_P4_28 24		// Port 4 pin 28 control
#define LPC_PINCON_PINMODE9_P4_29 26		// Port 4 pin 29 control

#define LPC_GPIOn_FIODIR_Pn_0 0
#define LPC_GPIOn_FIODIR_Pn_1 1
#define LPC_GPIOn_FIODIR_Pn_2 2
#define LPC_GPIOn_FIODIR_Pn_3 3
#define LPC_GPIOn_FIODIR_Pn_4 4
#define LPC_GPIOn_FIODIR_Pn_5 5
#define LPC_GPIOn_FIODIR_Pn_6 6
#define LPC_GPIOn_FIODIR_Pn_7 7
#define LPC_GPIOn_FIODIR_Pn_8 8
#define LPC_GPIOn_FIODIR_Pn_9 9
#define LPC_GPIOn_FIODIR_Pn_10 10
#define LPC_GPIOn_FIODIR_Pn_11 11
#define LPC_GPIOn_FIODIR_Pn_12 12
#define LPC_GPIOn_FIODIR_Pn_13 13
#define LPC_GPIOn_FIODIR_Pn_14 14
#define LPC_GPIOn_FIODIR_Pn_15 15
#define LPC_GPIOn_FIODIR_Pn_16 16
#define LPC_GPIOn_FIODIR_Pn_17 17
#define LPC_GPIOn_FIODIR_Pn_18 18
#define LPC_GPIOn_FIODIR_Pn_19 19
#define LPC_GPIOn_FIODIR_Pn_20 20
#define LPC_GPIOn_FIODIR_Pn_21 21
#define LPC_GPIOn_FIODIR_Pn_22 22
#define LPC_GPIOn_FIODIR_Pn_23 23
#define LPC_GPIOn_FIODIR_Pn_24 24
#define LPC_GPIOn_FIODIR_Pn_25 25
#define LPC_GPIOn_FIODIR_Pn_26 26
#define LPC_GPIOn_FIODIR_Pn_27 27
#define LPC_GPIOn_FIODIR_Pn_28 28
#define LPC_GPIOn_FIODIR_Pn_29 29
#define LPC_GPIOn_FIODIR_Pn_30 30
#define LPC_GPIOn_FIODIR_Pn_31 31






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


/**************************************************************
SSPn Control Register 0
LPC_SSPn->CR0
**************************************************************/

#define LPC_SSPn_CR0_DSS 0						// Data Size Select (Default: 0b0000)
#define LPC_SSPn_CR0_FRF 4						// Frame Format (Default: 0b00)
#define LPC_SSPn_CR0_CPOL 6						// Clock Out Polarity (Default: 0)
#define LPC_SSPn_CR0_CPHA 7						// Clock Out Phase (Default: 0)
#define LPC_SSPn_CR0_SCR 8						// Serial Clock Rate (Default: 0b00000000)


/**************************************************************
SSPn Control Register 1
LPC_SSPn->CR1
**************************************************************/

#define LPC_SSPn_CR1_LBM 0						// Loop Back Mode (Default: 0)
#define LPC_SSPn_CR1_SSE 1						// SSP Enable (Default: 0)
#define LPC_SSPn_CR1_MS 2						// Master/Slave Mode (Default: 0)
#define LPC_SSPn_CR1_SOD 3						// Slave Output Disable (Default: 0)


/**************************************************************
SPI Control Register
LPC_SPI->SPCR
**************************************************************/

#define LPC_SPI_SPCR_BitEnable 2				// (Default: 0)
#define LPC_SPI_SPCR_CPHA 3						// Clock phase control (Default: 0)
#define LPC_SPI_SPCR_CPOL 4						// Clock polarity control (Default: 0)
#define LPC_SPI_SPCR_MSTR 5						// Master mode select (Default: 0)
#define LPC_SPI_SPCR_LSBF 6						// LSB First controls (Default: 0)
#define LPC_SPI_SPCR_SPIE 7						// Serial peripheral interrupt enable (Default: 0)
#define LPC_SPI_SPCR_BITS 8						// bits per transfer (Default: 0b0000)


/**************************************************************
PWM Timer Control Register
LPC_PWM1->TCR
**************************************************************/

#define LPC_PWM1_TCR_CounterEnable 0			// The PWM Timer Counter and PWM Prescale Counter are enabled for counting. (Default: 0)
#define LPC_PWM1_TCR_CounterReset 1				// The PWM Timer Counter and the PWM Prescale Counter are synchronously reset on the next positive edge of PCLK. The counters remain reset until this bit is returned to zero. (Default: 0)
#define LPC_PWM1_TCR_PWMEnable 3				// PWM mode is enabled (Default: 0)


/**************************************************************
PWM Count Control Register
LPC_PWM1->CTCR
**************************************************************/

#define LPC_PWM1_CTCR_CTMode 0					// Counter / Timer Mode (Default: 0b00)
#define LPC_PWM1_CTCR_CISelect 2				// Count Input Select  (Default: 0b00)


/**************************************************************
PWM Match Control Register
LPC_PWM1->MCR
**************************************************************/

#define LPC_PWM1_MCR_PWMMR0I 0					// Interrupt on PWMMR0 (Default: 0)
#define LPC_PWM1_MCR_PWMMR0R 1					// Reset on PWMMR0 (Default: 0)
#define LPC_PWM1_MCR_PWMMR0S 2					// Stop on PWMMR0 (Default: 0)
#define LPC_PWM1_MCR_PWMMR1I 3					// Interrupt on PWMMR1 (Default: 0)
#define LPC_PWM1_MCR_PWMMR1R 4					// Reset on PWMMR1 (Default: 0)
#define LPC_PWM1_MCR_PWMMR1S 5					// Stop on PWMMR1 (Default: 0)
#define LPC_PWM1_MCR_PWMMR2I 6					// Interrupt on PWMMR2 (Default: 0)
#define LPC_PWM1_MCR_PWMMR2R 7					// Reset on PWMMR2 (Default: 0)
#define LPC_PWM1_MCR_PWMMR2S 8					// Stop on PWMMR2 (Default: 0)
#define LPC_PWM1_MCR_PWMMR3I 9					// Interrupt on PWMMR3 (Default: 0)
#define LPC_PWM1_MCR_PWMMR3R 10					// Reset on PWMMR3 (Default: 0)
#define LPC_PWM1_MCR_PWMMR3S 11					// Stop on PWMMR3 (Default: 0)
#define LPC_PWM1_MCR_PWMMR4I 12					// Interrupt on PWMMR4 (Default: 0)
#define LPC_PWM1_MCR_PWMMR4R 13					// Reset on PWMMR4 (Default: 0)
#define LPC_PWM1_MCR_PWMMR4S 14					// Stop on PWMMR4 (Default: 0)
#define LPC_PWM1_MCR_PWMMR5I 15					// Interrupt on PWMMR5 (Default: 0)
#define LPC_PWM1_MCR_PWMMR5R 16					// Reset on PWMMR5 (Default: 0)
#define LPC_PWM1_MCR_PWMMR5S 17					// Stop on PWMMR5 (Default: 0)
#define LPC_PWM1_MCR_PWMMR6I 18					// Interrupt on PWMMR6 (Default: 0)
#define LPC_PWM1_MCR_PWMMR6R 19					// Reset on PWMMR6 (Default: 0)
#define LPC_PWM1_MCR_PWMMR6S 20					// Stop on PWMMR6 (Default: 0)


/**************************************************************
PWM Control Register
LPC_PWM1->PCR
**************************************************************/

#define LPC_PWM1_PCR_PWMSEL2 2					// Single/Double-Edge-Mode for PWM2
#define LPC_PWM1_PCR_PWMSEL3 3					// Single/Double-Edge-Mode for PWM3
#define LPC_PWM1_PCR_PWMSEL4 4					// Single/Double-Edge-Mode for PWM4
#define LPC_PWM1_PCR_PWMSEL5 5					// Single/Double-Edge-Mode for PWM5
#define LPC_PWM1_PCR_PWMSEL6 6					// Single/Double-Edge-Mode for PWM6
#define LPC_PWM1_PCR_PWMENA1 9					// Disable/Enable PWM1 output
#define LPC_PWM1_PCR_PWMENA2 10					// Disable/Enable PWM2 output
#define LPC_PWM1_PCR_PWMENA3 11					// Disable/Enable PWM3 output
#define LPC_PWM1_PCR_PWMENA4 12					// Disable/Enable PWM4 output
#define LPC_PWM1_PCR_PWMENA5 13					// Disable/Enable PWM5 output
#define LPC_PWM1_PCR_PWMENA6 14					// Disable/Enable PWM6 output


/**************************************************************
PWM Latch Enable Register
LPC_PWM1->LER
**************************************************************/

#define LPC_PWM1_LER_MR0Latch 0					// Enable PWM Match 0 Latch
#define LPC_PWM1_LER_MR1Latch 1					// Enable PWM Match 1 Latch
#define LPC_PWM1_LER_MR2Latch 2					// Enable PWM Match 2 Latch
#define LPC_PWM1_LER_MR3Latch 3					// Enable PWM Match 3 Latch
#define LPC_PWM1_LER_MR4Latch 4					// Enable PWM Match 4 Latch
#define LPC_PWM1_LER_MR5Latch 5					// Enable PWM Match 5 Latch
#define LPC_PWM1_LER_MR6Latch 6					// Enable PWM Match 6 Latch










#endif