#ifndef __openDrive_UART_PORT_H 
#define __openDrive_UART_PORT_H


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


/* Hardware dependent functions */
uint32_t xUART2Port_Init(void);
void vUART2Port_SendChar(unsigned char SendChar);

/* Interrupt-Handler */
void UART2_IRQHandler(void);
void vUART2Port_ISR_LineStatus(void);
void vUART2Port_ISR_TimeOut(void);
void vUART2Port_ISR_SendChar(void);
void vUART2Port_ISR_ReceiveChar(void);



#endif /* end __openDrive_UART_PORT_H */