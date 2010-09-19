//
//  $Id: fiqISR.c 331 2008-11-09 16:59:47Z jcw $
//  $Revision: 331 $
//  $Author: jcw $
//  $Date: 2008-11-09 11:59:47 -0500 (Sun, 09 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/fiq/fiqISR.c $
//

#include "FreeRTOS.h"

#include "fiqISR.h"

//
//
//
volatile unsigned int fiqCounter;

//
//
//
void fiqISR (void) __attribute__ ((interrupt ("FIQ"))) __attribute__ ((noinline));
void fiqISR (void)
{
  ++fiqCounter;

  T1_IR = T_IR_MASK;
}

static void fiqISRNext (void) __attribute ((naked));
static void fiqISRNext (void)
{
}

unsigned int fiqISRLength (void)
{
  return (unsigned int) fiqISRNext - (unsigned int) fiqISR;
}
