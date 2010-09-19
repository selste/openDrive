//
//  $Id: uart1ISR.h 107 2008-10-10 19:54:55Z jcw $
//  $Revision: 107 $
//  $Author: jcw $
//  $Date: 2008-10-10 15:54:55 -0400 (Fri, 10 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uart/uart1ISR.h $
//

#ifndef _UART1ISR_H_
#define _UART1ISR_H_

#include "FreeRTOS.h"
#include "queue.h"

//
//
//
void uart1ISRCreateQueues (unsigned portBASE_TYPE uxQueueLength, xQueueHandle *pxRX0Queue, xQueueHandle *pxTX0Queue, portLONG volatile **pplTHREEmptyFlag);
void uart1ISR (void);

#endif
