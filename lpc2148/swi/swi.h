//
//  $Id: swi.h 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/swi/swi.h $
//

#ifndef _SWI_H_
#define _SWI_H_

//
//
//
typedef enum
{
  SWICALL_YIELDPROCESSOR = 0,   // FreeRTOS xYieldTask (asm)
  SWICALL_A_LED2SET,            // Set LED2 on or off, return previous state (asm)
  SWICALL_A_LED2ON,             // LED2 on, return previous state (asm)
  SWICALL_A_LED2OFF,            // LED2 off, return previous state (asm)
  SWICALL_A_LED2TOGGLE,         // LED2 toggle, return previous state (asm)
  SWICALL_C_LED2SET,            // Set LED2 on or off, return previous state (C)
  SWICALL_C_LED2ON,             // LED2 on, return previous state (C)
  SWICALL_C_LED2OFF,            // LED2 off, return previous state (C)
  SWICALL_C_LED2TOGGLE,         // LED2 toggle, return previous state (C)
}
swiCalls_e;

//
//
//
#define SWICALL(swiID, swiOut) \
	asm  volatile (\
	"swi %a1   \n\t" \
	"mov %0,r0 \n\t" \
	: "=r" (swiOut) : "I" (swiID) : "r0", "lr")

#define SWICALL1(swiID, swiIn, swiOut) \
	asm  volatile (\
	"mov r0,%1 \t\n" \
	"swi %a2   \n\t" \
	"mov %0,r0 \n\t" \
	: "=r" (swiOut) : "r" (swiIn), "I" (swiID) : "r0", "lr")

#define SWICALL2(swiID, swiIn1, swiIn2, swiOut) \
	asm volatile ( \
	"mov r0,%1 \t\n" \
	"mov r1,%2 \t\n" \
	"swi %a3   \n\t" \
	"mov %0,r0 \n\t" \
	: "=r" (swiOut) : "r" (swiIn1), "r" (swiIn2), "I" (swiID) : "r0", "lr")

#define SWICALL3(swiID, swiIn1, swiIn2, swiIn3, swiOut) \
	asm volatile ( \
	"mov r0,%1 \t\n" \
	"mov r1,%2 \t\n" \
	"mov r2,%3 \t\n" \
	"swi %a4   \n\t" \
	"mov %0,r0 \n\t" \
	: "=r" (swiOut) : "r" (swiIn1), "r" (swiIn2), "r" (swiIn3), "I" (swiID) : "r0", "lr")

//
//
//
void swiInit (void);
int swiDispatchC (unsigned long r0, unsigned long r1, unsigned long r2, unsigned long swi);

#endif
