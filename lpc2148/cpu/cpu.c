//
//  $Id: cpu.c 356 2009-01-12 04:11:31Z jcw $
//  $Revision: 356 $
//  $Author: jcw $
//  $Date: 2009-01-11 23:11:31 -0500 (Sun, 11 Jan 2009) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/cpu/cpu.c $
//

#include "FreeRTOS.h"

#include "../fiq/fiq.h"
#include "cpu.h"

//
//
//
typedef struct intVects_s
{
  unsigned int reset;
  unsigned int undefined;
  unsigned int swi;
  unsigned int pabt;
  unsigned int dabt;
  unsigned int reserved;
  unsigned int irq;
  unsigned int fiq;

  void *reset_handler;
  void *undefined_handler;
  void *swi_handler;
  void *pabt_handler;
  void *dabt_handler;
  void *reserved_handler;
  void *irq_handler;
  void *fiq_handler;
}
__attribute__ ((packed)) intVects_t;

//
//
//
extern unsigned int __intvects;

//
//
//
void cpuSetupHardware (void)
{
#ifdef RUN_FROM_RAM
  //
  //  Remap the interrupt vectors to RAM if we are are running from RAM
  //
  SCB_MEMMAP = SCB_MEMMAP_URM;
#endif

  //
  //  Use fast I/O on both ports
  //
  SCB_SCS = SCB_SCS_GPIO0M | SCB_SCS_GPIO1M;

  //
  //  Configure pin functions.  All pins are set to GPIO, including the Debug
  //  port (P1.26) and the Trace port (P1.25..P1.16).
  //
  PCB_PINSEL0 = PCB_PINSEL0_ALL_GPIO;
  PCB_PINSEL1 = PCB_PINSEL1_ALL_GPIO;
  PCB_PINSEL2 = (PCB_PINSEL2_P13626_GPIO | PCB_PINSEL2_P12516_GPIO);

  //
  //  Set all GPIO to input (safer than an output, which may be driving a high
  //  into a closed switch such as BSL).  
  //
  GPIO0_FIODIR = ~GPIO_IO_ALL;
  GPIO1_FIODIR = ~GPIO_IO_JTAG;

  //
  //  Setup the PLL to multiply the 12Mhz XTAL input by 4, divide by 1
  //
  SCB_PLLCFG = (SCB_PLLCFG_MUL4 | SCB_PLLCFG_DIV1);

  //
  //  Activate the PLL by turning it on then feeding the correct sequence of bytes
  //
  SCB_PLLCON  = SCB_PLLCON_PLLE;
  SCB_PLLFEED = SCB_PLLFEED_FEED1;
  SCB_PLLFEED = SCB_PLLFEED_FEED2;

  //
  //  Wait for the PLL to lock...
  //
  while (!(SCB_PLLSTAT & SCB_PLLSTAT_PLOCK))
    ;

  //
  //  ...before connecting it using the feed sequence again
  //
  SCB_PLLCON  = SCB_PLLCON_PLLC | SCB_PLLCON_PLLE;
  SCB_PLLFEED = SCB_PLLFEED_FEED1;
  SCB_PLLFEED = SCB_PLLFEED_FEED2;

  //
  //  Setup and turn on the MAM.  Three cycle access is used due to the fast
  //  PLL used.  It is possible faster overall performance could be obtained by
  //  tuning the MAM and PLL settings.
  //
  MAM_TIM = MAM_TIM_3;
  MAM_CR = MAM_CR_FULL;

  //
  //  Setup the peripheral bus to be the same as the PLL output (48Mhz)
  //
  SCB_VPBDIV = SCB_VPBDIV_100;

  //
  //  Disable power to all modules
  //
  SCB_PCONP = SCB_PCONP_ALLOFF;

  //
  //  Make sure all interrupts disabled
  //
  VIC_IntEnClr = VIC_IntEnClr_MASK;

  //
  //  Put FIQ handler in RAM.  If there wasn't enough space (because malloc()
  //  failed, and that would be *really* unusual), cpuSetupFIQISR() won't be
  //  called by fiqFIQISRCopyToRAM(), so the FIQ handler will remain in flash.
  //  If CFG_RAM_INTS is not defined, the FIQ handler be executed from flash,
  //  not RAM, because the FIQ vector will point to the version in flash, not
  //  in RAM.  It'll all still work, but just slower.
  //
#ifdef CFG_FIQ
  fiqFIQISRCopyToRAM ();
#endif

#ifdef CFG_RAM_INTS
  //
  //  Lastly, switch interrupt handlers to RAM vectors
  //
  SCB_MEMMAP = SCB_MEMMAP_URM;
#endif
}

//
//  If CFG_RAM_INTS is not defined, FIQ vector will be in the flash interrupt 
//  vector table (in boot.s), and the FIQ handler code copied to RAM will not
//  not be used.  The FIQ will still run, but will be slower.
//
void cpuSetupFIQISR (void *FIQInterrupt)
{
#ifdef CFG_RAM_INTS
  intVects_t *ivRam = (intVects_t *) &__intvects;

  ivRam->fiq_handler = FIQInterrupt;
#endif
}

//
//
//
void cpuPLLDisable (void)
{
  SCB_PLLCON  = 0;
  SCB_PLLFEED = SCB_PLLFEED_FEED1;
  SCB_PLLFEED = SCB_PLLFEED_FEED2;
  SCB_PLLCFG =  0;
}

//
//
//
void cpuT1Disable (void)
{
  T1_TCR = 0;
  T1_PR = 0;
  T1_MCR = 0;
  T1_CCR = 0;
  T1_EMR = 0;
  T1_CTCR = 0;
  T1_IR = 0;
}
