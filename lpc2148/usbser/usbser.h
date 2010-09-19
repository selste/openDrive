//
//  $Id: usbser.h 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usbser/usbser.h $
//

#ifndef _USBSER_H_
#define _USBSER_H_

#include "FreeRTOS.h"
#include "queue.h"

//
//
//
signed portBASE_TYPE usbserPutChar (signed portCHAR cOutChar, portTickType xBlockTime);
signed portBASE_TYPE usbserPutString (const signed portCHAR * const pcString, portTickType xBlockTime);
signed portBASE_TYPE usbserGetChar (signed portCHAR *pcRxedChar, portTickType xBlockTime);
void usbserInit (void);
void usbserGetRxQueue (xQueueHandle *qh);

#endif
