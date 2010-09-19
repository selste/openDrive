//
//  $Id: eint1ISR.c 128 2008-10-11 14:54:27Z jcw $
//  $Revision: 128 $
//  $Author: jcw $
//  $Date: 2008-10-11 10:54:27 -0400 (Sat, 11 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/eints/eint1ISR.c $
//

#include "FreeRTOS.h"

#include "eint1ISR.h"

//
//
//
static void eint1ISR_Handler (void)
{
  SCB_EXTINT |= SCB_EXTINT_EINT1;

	VIC_VectAddr = (unsigned portLONG) 0;
}

void eint1ISR (void) __attribute__ ((naked));
void eint1ISR (void)
{
  portSAVE_CONTEXT ();
  eint1ISR_Handler ();
  portRESTORE_CONTEXT ();
}
