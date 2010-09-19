@
@  $Id: swidispatch.s 59 2008-10-05 03:43:08Z jcw $
@  $Revision: 59 $
@  $Author: jcw $
@  $Date: 2008-10-04 23:43:08 -0400 (Sat, 04 Oct 2008) $
@  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/swi/swidispatch.s $
@

@
@  This code expects to find a function named 'swiDispatchC' (which should be
@  in swi.c) to dispatch any functions that aren't written in assembly.
@  (What nut-case picked '@' as the comment character for .s files?)
@
        .text
        .extern swiDispatchC
        .global swiDispatch
        .func   swiDispatch

@
@  Equates for GPIO0 registers
@
        .set    GPIO0_FIODIR, 0x3fffc000
        .set    GPIO0_FIOPIN, 0x3fffc014
        .set    GPIO0_FIOSET, 0x3fffc018
        .set    GPIO0_FIOCLR, 0x3fffc01c
        .set    GPIO_IO_P11, 0x00000800

@
@  No check for Thumb mode.  The LPC2148 demo code doesn't do Thumb
@
swiDispatch:
        stmfd   sp!, {r4, lr}           @ Save r4, lr
        ldr     r4, [lr, #-4]           @ Fetch the SWI instruction that invoked us
        bic     r4, r4, #0xff000000     @ Clear top 8 bits leaving swi "comment field"=number
        cmp     r4, #SWI_LAST           @ <= SWI_LAST, handled in assembly, otherwise, in C handler
        ldrls   pc, [pc, r4, lsl #2]    @ If <= SWI_LAST, we're handling it in assembly
        b       swiDispatchToC          @ Not handled in here, switch to the C handler
        
@ 
@  Dispatch table for functions in assembly.  The swiEntriesStart label isn't
@  referenced directly by the code.  Rather, it must directly follow the 
@  swiDispatch code, because it is relatively referenced.
@
swiEntriesStart:
        .word   swiPYP                  @ 0 - FreeRTOS Yield Process
        .word   LED2Set                 @ 1 - Set LED2 state, return previous state
        .word   LED2On                  @ 2 - Turn LED2 on, return previous state
        .word   LED2Off                 @ 3 - Turn LED2 off, return previous state
        .word   LED2Toggle              @ 4 - Toggle LED2, return previous state
swiEntriesEnd:
        .set    SWI_LAST, ((swiEntriesEnd-swiEntriesStart)/4)-1

@
@  FreeRTOS uses SWI to yield process, but it's greedy and wants SWI all to its 
@  self.  So... Special handling it is!
@
swiPYP:
        ldmfd   sp!, {r4, lr}           @ Recover r4, lr
        b       vPortYieldProcessor     @ Yield... yield like the wind!

@
@  Values are returned in r0.  LED2 is hardwired in this code (bad programmer!)  Also
@  remember that LED2 (and LED1) have their anodes tied to +3.3V, so we have to drive
@  the port pin LOW to turn them on.  When the caller wants to turn an LED off, it 
@  passes in a non-zero value (keeping the same software logic as the hardware logic)
@
LED2Set:
        stmfd   sp!, {r1}               @ Save r1
        cmp     r0, #0                  @ Turn LED2 off?
        ldreq   r4, =GPIO0_FIOCLR       @ Get GPIO0_FIOCLR address (0 == on)
        ldrne   r4, =GPIO0_FIOSET       @ Get GPIO0_FIOSET address (!0 == off)
        ldr     r0, =GPIO0_FIOPIN       @ Load GPIO0_FIOPIN address
        ldr     r0, [r0]                @ Get GPIO0_FIOPIN contents
        ldr     r1, =GPIO_IO_P11        @ Constant for port pin LED2 attached to
        and     r0, r0, r1              @ R0 has bit that reflects LED2 current state
        str     r1, [r4]                @ Set LED2 on
        ldmfd   sp!, {r1}               @ Recover r1
        b       swiExit                 @ Exit SWI handler

LED2On:
        stmfd   sp!, {r1}               @ Save r1
        ldr     r0, =GPIO0_FIOPIN       @ Load GPIO0_FIOPIN address
        ldr     r0, [r0]                @ Get GPIO0_FIOPIN contents
        ldr     r1, =GPIO_IO_P11        @ Constant for port pin LED2 attached to
        and     r0, r0, r1              @ R0 has bit that reflects LED2 current state
        ldr     r4, =GPIO0_FIOCLR       @ Get GPIO0_FIOCLR address
        str     r1, [r4]                @ Set LED2 on
        ldmfd   sp!, {r1}               @ Recover r1
        b       swiExit                 @ Exit SWI handler

LED2Off:
        stmfd   sp!, {r1}               @ Save r1
        ldr     r0, =GPIO0_FIOPIN       @ Load GPIO0_FIOPIN address
        ldr     r0, [r0]                @ Get GPIO0_FIOPIN contents
        ldr     r1, =GPIO_IO_P11        @ Constant for port pin LED2 attached to
        and     r0, r0, r1              @ R0 has bit that reflects LED2 current state
        ldr     r4, =GPIO0_FIOSET       @ Get GPIO0_FIOSET address
        str     r1, [r4]                @ Set LED2 off
        ldmfd   sp!, {r1}               @ Recover r1
        b       swiExit                 @ Exit SWI handler

LED2Toggle:
        stmfd   sp!, {r1}               @ Save r1
        ldr     r0, =GPIO0_FIOPIN       @ Load GPIO0_FIOPIN address
        ldr     r0, [r0]                @ Get GPIO0_FIOPIN contents
        ldr     r1, =GPIO_IO_P11        @ Constant for port pin LED2 attached to
        ands    r0, r0, r1              @ R0 has bit that reflects LED2 current state
        ldrne   r4, =GPIO0_FIOCLR       @ Get GPIO0_FIOCLR address (!0 == off, so turn on)
        ldreq   r4, =GPIO0_FIOSET       @ Get GPIO0_FIOSET address (0 == on, so turn off)
        str     r1, [r4]                @ Set LED2 off
        ldmfd   sp!, {r1}               @ Recover r1
        b       swiExit                 @ Exit SWI handler

@
@  Not an internal function, try the C dispatch handler.  r0, r1, r2 will be
@  passed through as set.  r3 will contain the SWI number.  Return value will
@  be in r0.
@
swiDispatchToC:
        stmfd   sp!,{r1-r12, lr}        @ Save r1..r12, lr on stack
        mrs     r12, spsr               @ Saved Process Status Register (SPSR) to r12
        stmfd   sp!, {r12}              @ Save r12
        ldr     r12, =swiDispatchC      @ Get the address of the C handler function
        mov     r3, r4                  @ Pass SWI number as parameter in r3
        mov     lr, pc                  @ Save current PC to lr
        bx      r12                     @ Branch to r12 (C handler function), return to next instruction
        ldmfd   sp!, {r12}              @ Recover old SPSR to r12
        msr     spsr_cxsf, r12          @ Set SPSR from r12
        ldmfd   sp!, {r1-r12, pc}^      @ Recover r1..r12, return value in r0
        b       swiExit                 @ And exit

@
@  Exit SWI handler.  Recover r4, return to caller
@
swiExit:
        ldmfd   sp!, {r4, pc}^          @ Recover r4, return to caller

        .endfunc
        .end
