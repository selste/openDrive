//
//  $Id: fiq.c 355 2009-01-12 04:10:58Z jcw $
//  $Revision: 355 $
//  $Author: jcw $
//  $Date: 2009-01-11 23:10:58 -0500 (Sun, 11 Jan 2009) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/fiq/fiq.c $
//

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"

#include "../cpu/cpu.h"
#include "fiqISR.h"
#include "fiq.h"

//
//
//
extern volatile unsigned int fiqCounter;

//
//
//
void fiqInit (void)
{
  SCB_PCONP |= SCB_PCONP_PCTIM1;

  VIC_IntSelect |= VIC_IntSelect_Timer1;
  VIC_IntEnable = VIC_IntEnable_Timer1;

  T1_PR = 0;
  T1_MR0 = configCPU_CLOCK_HZ / 8;
  T1_MCR = T_MCR_MR0R | T_MCR_MR0I;
}

int fiqEnable (void)
{
  unsigned int state = T1_TCR;

  //
  //  Only needed in case someone used the 'beep' command, which also use timer 1.
  //
  fiqInit ();

  T1_TCR = T_TCR_CE;

  return (state & T_TCR_CE) ? 1 : 0;
}

int fiqDisable (void)
{
  unsigned int state = T1_TCR;

  T1_TCR = T_TCR_CR;

  return (state & T_TCR_CE) ? 1 : 0;
}

unsigned int fiqGetCount (void)
{
  return fiqCounter;
}

void fiqClearCount (void)
{
  fiqCounter = 0;
}

unsigned char *fiqFIQISRCopyToRAM (void)
{
  static unsigned char *FIQInterrupt = NULL;

  if (!FIQInterrupt)
  {
    if ((FIQInterrupt = malloc (fiqISRLength ())))
    {
      memcpy (FIQInterrupt, &fiqISR, fiqISRLength ());
      cpuSetupFIQISR (FIQInterrupt);
    }
  }

  return FIQInterrupt;
}

#if 0
//
//  If you prefer a version that keeps the interrupt vectors in a static
//  variable instead of malloc(), enable this version, and disable the one
//  above.  
//
unsigned char *fiqFIQISRCopyToRAM (void)
{
  static unsigned int FIQInterrupt [16];

  if (sizeof (FIQInterrupt) >= fiqISRLength ())
  {
    memcpy (FIQInterrupt, &fiqISR, fiqISRLength ());
    cpuSetupFIQISR (FIQInterrupt);
  }

  return (unsigned char *) &FIQInterrupt [0];
}
#endif
