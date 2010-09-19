//
//  $Id: eint3.c 159 2008-10-17 01:32:05Z jcw $
//  $Revision: 159 $
//  $Author: jcw $
//  $Date: 2008-10-16 21:32:05 -0400 (Thu, 16 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/eints/eint3.c $
//

#include "FreeRTOS.h"

#include "eint3.h"
#include "eint3ISR.h"

//
//  Vector 3 will either be UART 1 or EINT3, but both will never be active at the same time
//
void eint3Init (void)
{
  portENTER_CRITICAL ();

  PCB_PINSEL0 = (PCB_PINSEL0 & ~PCB_PINSEL0_P09_MASK) | PCB_PINSEL0_P09_EINT3;

  //
  //  Set for low-active/falling-edge sensitive & edge sensitive, then clear interrupt
  //
  SCB_EXTPOLAR &= ~SCB_EXTPOLAR_EINT3;
  SCB_EXTMODE  |=  SCB_EXTMODE_EINT3;
  SCB_EXTINT   |=  SCB_EXTINT_EINT3;

  VIC_IntSelect &= ~VIC_IntSelect_EINT3;
  VIC_VectAddr3 = (portLONG) eint3ISR;
  VIC_VectCntl3 = VIC_VectCntl_ENABLE | VIC_Channel_EINT3;
  VIC_IntEnable = VIC_IntEnable_EINT3;

  portEXIT_CRITICAL ();
}

void eint3Deinit (void)
{
  VIC_IntEnClr = VIC_IntEnClr_EINT3;

  PCB_PINSEL0 = (PCB_PINSEL0 & ~PCB_PINSEL0_P09_MASK) | PCB_PINSEL0_P09_GPIO;
}
