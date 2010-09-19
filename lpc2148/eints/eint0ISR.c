//
//  $Id: eint0ISR.c 128 2008-10-11 14:54:27Z jcw $
//  $Revision: 128 $
//  $Author: jcw $
//  $Date: 2008-10-11 10:54:27 -0400 (Sat, 11 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/eints/eint0ISR.c $
//

#include "FreeRTOS.h"

#include "eint0ISR.h"

//
//
//
static void eint0ISR_Handler (void)
{
  SCB_EXTINT |= SCB_EXTINT_EINT0;

  GPIO0_FIOSET = GPIO_IO_P11;

	VIC_VectAddr = (unsigned portLONG) 0;
}

void eint0ISR (void) __attribute__ ((naked));
void eint0ISR (void)
{
  portSAVE_CONTEXT ();
  eint0ISR_Handler ();
  portRESTORE_CONTEXT ();
}
