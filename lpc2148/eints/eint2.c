//
//  $Id: eint2.c 159 2008-10-17 01:32:05Z jcw $
//  $Revision: 159 $
//  $Author: jcw $
//  $Date: 2008-10-16 21:32:05 -0400 (Thu, 16 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/eints/eint2.c $
//

#include "FreeRTOS.h"

#include "eint2.h"
#include "eint2ISR.h"

//
//
//
void eint2Init (void)
{
  portENTER_CRITICAL ();

  PCB_PINSEL0 = (PCB_PINSEL0 & ~PCB_PINSEL0_P015_MASK) | PCB_PINSEL0_P015_EINT2;

  SCB_EXTPOLAR &= ~SCB_EXTPOLAR_EINT2;
  SCB_EXTMODE  |=  SCB_EXTMODE_EINT2;
  SCB_EXTINT   |=  SCB_EXTINT_EINT2;

  VIC_IntSelect &= ~VIC_IntSelect_EINT2;
  VIC_VectAddr5 = (portLONG) eint2ISR;
  VIC_VectCntl5 = VIC_VectCntl_ENABLE | VIC_Channel_EINT2;
  VIC_IntEnable = VIC_IntEnable_EINT2;

  portEXIT_CRITICAL ();
}

void eint2Deinit (void)
{
  VIC_IntEnClr = VIC_IntEnClr_EINT2;

  PCB_PINSEL0 = (PCB_PINSEL0 & ~PCB_PINSEL0_P015_MASK) | PCB_PINSEL0_P015_GPIO;
}
