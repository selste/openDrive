//
//  $Id: cpu.h 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/cpu/cpu.h $
//

#ifndef _CPU_H_
#define _CPU_H_

void cpuSetupHardware (void);
void cpuSetupFIQISR (void *FIQInterrupt);
void cpuPLLDisable (void);
void cpuT1Disable (void);
void cpuGPIOInitialize (void);
void cpuToggleLED (unsigned portBASE_TYPE uxLED);

#endif
