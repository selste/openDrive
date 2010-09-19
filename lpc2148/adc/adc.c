//
//  $Id: adc.c 188 2008-10-19 14:36:39Z jcw $
//  $Revision: 188 $
//  $Author: jcw $
//  $Date: 2008-10-19 10:36:39 -0400 (Sun, 19 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/adc/adc.c $
//

#include "FreeRTOS.h"

#include "adc.h"

//
//
//
int adcRead0_3 (void)
{
  AD0_CR &= ~(AD_CR_START_MASK | AD_CR_SELMASK);
  AD0_CR |=  (AD_CR_START_NOW | AD_CR_SEL3);
  AD0_CR |=   AD_CR_START_NOW;

  while (!(AD0_DR3 & AD_DR_DONE))
    ;

  return ((AD0_DR3 & AD_DR_RESULTMASK) >> AD_DR_RESULTSHIFT);
}

//
//  Assumes PCLK == 48Mhz
//
void adcInit (void)
{
  PCB_PINSEL1 = (PCB_PINSEL1 & ~PCB_PINSEL1_P030_MASK) | PCB_PINSEL1_P030_AD03;

  SCB_PCONP |= SCB_PCONP_PCAD0;

  AD0_CR = AD_CR_CLKS10 | AD_CR_PDN | ((11 - 1) << AD_CR_CLKDIVSHIFT) | AD_CR_SEL3;
}
