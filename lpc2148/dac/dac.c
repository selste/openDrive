//
//  $Id: dac.c 188 2008-10-19 14:36:39Z jcw $
//  $Revision: 188 $
//  $Author: jcw $
//  $Date: 2008-10-19 10:36:39 -0400 (Sun, 19 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/dac/dac.c $
//

#include "FreeRTOS.h"

#include "dac.h"

//
//
//
void dacInit (void)
{
  PCB_PINSEL1 = (PCB_PINSEL1 & ~PCB_PINSEL1_P025_MASK) | PCB_PINSEL1_P025_AOUT;

  DAC_CR = 0;
}

unsigned int dacSet (unsigned int newValue)
{
  unsigned int dacCR;
  unsigned int dacCurrentValue;

  dacCR = DAC_CR;
  dacCurrentValue = (dacCR & DAC_CR_VALUEMASK) >> DAC_CR_VALUESHIFT;
  dacCR = (dacCR & ~DAC_CR_VALUEMASK) | ((newValue << DAC_CR_VALUESHIFT) & DAC_CR_VALUEMASK);
  DAC_CR = dacCR;

  return dacCurrentValue;
}
