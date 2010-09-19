//
//  $Id: eint2ISR.c 128 2008-10-11 14:54:27Z jcw $
//  $Revision: 128 $
//  $Author: jcw $
//  $Date: 2008-10-11 10:54:27 -0400 (Sat, 11 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/eints/eint2ISR.c $
//

#include "FreeRTOS.h"

#include "eint2ISR.h"

//
//
//
static void eint2ISR_Handler (void)
{
  SCB_EXTINT |= SCB_EXTINT_EINT2;

  GPIO0_FIOCLR = GPIO_IO_P11;

	VIC_VectAddr = (unsigned portLONG) 0;
}

void eint2ISR (void) __attribute__ ((naked));
void eint2ISR (void)
{
  portSAVE_CONTEXT ();
  eint2ISR_Handler ();
  portRESTORE_CONTEXT ();
}

