@
@  $Id: boot.s 354 2009-01-12 04:09:59Z jcw $
@  $Revision: 354 $
@  $Author: jcw $
@  $Date: 2009-01-11 23:09:59 -0500 (Sun, 11 Jan 2009) $
@  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/boot.s $
@

@
@  Sample initialization file 
@
        .extern main
        .extern exit

        .text
        .code 32

        .align  0

        .extern __bss_beg__
        .extern __bss_end__
        .extern __stack_end__
        .extern __data_beg__
        .extern __data_beg_src__   
        .extern __intvects_beg__     
        .extern __intvects_beg_src__
        .extern __intvects_end__   
        .extern __data_end__
        .extern __data+beg_src__

        .global start
        .global endless_loop

@ 
@  Stack sizes.  These have been determined empirically.  If your interrupt
@  routines become more complex or use a lot of dynamically allocated space,
@  the IRQ and/or FIQ stacks may been be grown.  The supervisor stack may 
@  overflow into the system/user stack as the system is going any initialization,
@  before starting the FreeRTOS scheduler.  The scheduler itself needs a small
@  amount of supervisor stack space, once it's running.
@
        .set  UND_STACK_SIZE, 0x00000004
        .set  ABT_STACK_SIZE, 0x00000004
        .set  FIQ_STACK_SIZE, 0x00000020
        .set  IRQ_STACK_SIZE, 0X00000100
        .set  SVC_STACK_SIZE, 0x00000100

@
@  Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs 
@
        .set  MODE_USR, 0x10            @ User Mode 
        .set  MODE_FIQ, 0x11            @ FIQ Mode 
        .set  MODE_IRQ, 0x12            @ IRQ Mode 
        .set  MODE_SVC, 0x13            @ Supervisor Mode 
        .set  MODE_ABT, 0x17            @ Abort Mode 
        .set  MODE_UND, 0x1B            @ Undefined Mode 
        .set  MODE_SYS, 0x1F            @ System Mode 

        .equ  I_BIT, 0x80               @ when I bit is set, IRQ is disabled 
        .equ  F_BIT, 0x40               @ when F bit is set, FIQ is disabled 
        .equ  T_BIT, 0x20               @ when T bit is set, THUMB mode is active

start:
__start:
_mainCRTStartup:
@
@  Clear all of memory to 0xe5e5e5e5.  We use this value later to determine
@  stack highwater usage.
@
        ldr     r1, =__data_beg__       @ __data_beg__ is start of RAM 
        ldr     r3, =__stack_end__      @ __stack_end__ is end of RAM
        sub     r3, r3, r1              @ Length of RAM to set
        ldr     r2, =0xe5e5e5e5         @ Fill value

.init_loop:
        str     r2, [r1], #4            @ Store fill value, r1 += 4
        subs    r3, r3, #4              @ Length -= 4
        bgt     .init_loop              @ >= 0, go again

.end_init_loop:

@
@  Clear BSS. 
@
        ldr     r1, =__bss_beg__        @ Start of memory block 
        ldr     r3, =__bss_end__        @ End of memory block 
        subs    r3, r3, r1              @ Calculate length of block 
        beq     .end_clear_loop         @ If 0, nothing to do
        mov     r2, #0                  @ Fill value

.clear_loop:
        strb    r2, [r1], #1            @ Store byte, r1++
        subs    r3, r3, #1              @ Decrement counter
        bgt     .clear_loop             @ >= 0, go again

.end_clear_loop:

@
@  Initialize data.
@
        ldr     r1, =__data_beg__       @ Destination (.data in RAM)
        ldr     r2, =__data_beg_src__   @ Source (.data in FLASH)
        ldr     r3, =__data_end__       @ End of .data in RAM
        subs    r3, r3, r1              @ Calculate length of block 
        beq     .end_set_loop           @ If 0, nothing to do

.set_loop:
        ldrb    r4, [r2], #1            @ Get byte from source, r2++
        strb    r4, [r1], #1            @ Store byte to destination, r1++
        subs    r3, r3, #1              @ Decrement loop counter
        bgt     .set_loop               @ >= 0, go again

.end_set_loop:

@
@  Copy interrupt vectors (See note below why this is not disabled with 
@  CFG_RAM_INTS isn't defined)
@
        ldr     r1, =__intvects_beg__     @ Destination (.intvects in RAM)
        ldr     r2, =__intvects_beg_src__ @ Source (.intvects in FLASH)
        ldr     r3, =__intvects_end__     @ End of .intvects in RAM
        subs    r3, r3, r1                @ Calculate length of block 
        beq     .end_ints_loop            @ If 0, nothing to do

.ints_loop:
        ldrb    r4, [r2], #1            @ Get byte from source, r2++
        strb    r4, [r1], #1            @ Store byte to destination, r1++
        subs    r3, r3, #1              @ Decrement loop counter
        bgt     .ints_loop              @ >= 0, go again

.end_ints_loop:

@
@  Setup a stack for each mode - note that this only sets up a usable stack
@  for system/user, SWI and IRQ modes.   Also each mode is setup with
@  interrupts initially disabled. 
@
        ldr   r1, =__stack_beg_und          @ Pointer to various values we update
        ldr   r0, =__stack_end__            @ Get top of stack space
        msr   CPSR_c, #MODE_UND|I_BIT|F_BIT @ Undefined Instruction Mode
        mov   sp, r0                        @ Set undef mode SP
        str   r0, [r1, #0]                  @ Store this so 'mem map' knows

        sub   r0, r0, #UND_STACK_SIZE       @ Subtract undef stack size for abort stack start
        msr   CPSR_c, #MODE_ABT|I_BIT|F_BIT @ Abort Mode 
        mov   sp, r0                        @ Set abort mode SP
        str   r0, [r1, #4]                  @ Store this so 'mem map' knows
        str   r0, [r1, #8]                  @ Store this so 'mem map' knows

        sub   r0, r0, #ABT_STACK_SIZE       @ Subtract abort stack size for FIQ stack start
        msr   CPSR_c, #MODE_FIQ|I_BIT|F_BIT @ FIQ Mode 
        mov   sp, r0                        @ Set FIQ mode SP
        str   r0, [r1, #12]                 @ Store this so 'mem map' knows
        str   r0, [r1, #16]                 @ Store this so 'mem map' knows

        sub   r0, r0, #FIQ_STACK_SIZE       @ Subtract FIQ stack size for IRQ stack start
        msr   CPSR_c, #MODE_IRQ|I_BIT|F_BIT @ IRQ Mode 
        mov   sp, r0                        @ Set IRQ mode SP
        str   r0, [r1, #20]                 @ Store this so 'mem map' knows
        str   r0, [r1, #24]                 @ Store this so 'mem map' knows

        sub   r0, r0, #IRQ_STACK_SIZE       @ Subtract IRQ stack size for SVC stack start
        msr   CPSR_c, #MODE_SVC|I_BIT|F_BIT @ Supervisor Mode 
        mov   sp, r0                        @ Set supervisor mode SP
        str   r0, [r1, #28]                 @ Store this so 'mem map' knows
        str   r0, [r1, #32]                 @ Store this so 'mem map' knows
        sub   r2, r0, #256                  @ MAGIC! FreeRTOS only uses a few bytes of supervisor stack...
        str   r2, [r1, #48]                 @ ...so tell _sbrk() where heap ends when FreeRTOS running

        sub   r0, r0, #SVC_STACK_SIZE       @ Subtract supervisor stack size for system/user stack start
        msr   CPSR_c, #MODE_SYS|I_BIT|F_BIT @ System Mode 
        mov   sp, r0                        @ Set system/user mode SP
        str   r0, [r1, #36]                 @ Store this so 'mem map' knows
        str   r0, [r1, #40]                 @ Store this so 'mem map' knows

@
@  We want to start in supervisor mode (probably always, but FreeRTOS demands it)
@
        msr   CPSR_c, #MODE_SVC|I_BIT|F_BIT

@
@  Set argc & argv, initialize newlib, and jump to main
@
        mov     r0, #0                  @ No arguments  
        mov     r1, #0                  @ No argv either 

        bl      syscallsInit            @ Initialize ./newlib/syscalls.c
        bl      main                    @ And call good ol' main()
        b       .                       @ In case main() ever returns

@
@  Setup vector table.  
@
        .section .startup,"ax"
        .code 32
        .align 0

        ldr     pc, _start              @ reset - _start               
        ldr     pc, _undf               @ undefined - _undf            
        ldr     pc, _swi                @ SWI - _swi                   
        ldr     pc, _pabt               @ program abort - _pabt        
        ldr     pc, _dabt               @ data abort - _dabt           
        ldr     pc, _start              @ reserved - reset
        ldr     pc, [pc, #-0xff0]       @ IRQ - read VIC_VectAddr (magic!  It's at 0xfffff030)
        ldr     pc, _fiq                @ FIQ - _fiq                   

_start: .word   __start                 @ reset
_undf:  .word   __undf                  @ undefined                    
_swi:   .word   swiDispatch             @ SWI
_pabt:  .word   __pabt                  @ program abort                
_dabt:  .word   __dabt                  @ data abort                   
_fiq:   .word   fiqISR                  @ FIQ                          

@
@  Handlers for undef, program abort and data abort.  They all update
@  their respective registers, then reset the system by timing out
@  the watchdog (only apparent way to force a hardware reset)
@
__undf: 
        ldr     sp, =(__abort_mem+5*4)  @ Set sp_abt to data array with offset (restore later)
        stmia   sp, {r0-r12}            @ Save first dataset in r0-r12 to array
        sub     r0, lr, #4              @ Calculate PC value of undef instruction
        mov     r1, #0                  @ Abort type
        b       .abtstore               @ Save info, reset system

__pabt: 
        ldr     sp, =(__abort_mem+5*4)  @ Set sp_abt to data array with offset (restore later)
        stmia   sp, {r0-r12}            @ Save first dataset in r0-r12 to array
        sub     r0, lr, #4              @ Calculate PC value of undef instruction
        mov     r1, #1                  @ Abort type
        b       .abtstore               @ Save info, reset system

__dabt: 
        ldr     sp, =(__abort_mem+5*4)  @ Set sp_abt to data array with offset (restore later)
        stmia   sp, {r0-r12}            @ Save first dataset in r0-r12 to array
        sub     r0, lr, #8              @ Calculate PC value of undef instruction
        mov     r1, #2                  @ Abort type
        b       .abtstore               @ Save info, reset system

@
@  Store the abort type.  Then see if the sigil value is set, and if not,
@  reset the abort counter to 0.
@
.abtstore:
        ldr     r2, =__abort_typ        @ Abort type
        str     r1, [r2]                @ Store it

        ldr     r2, =__abort_sig        @ Get the sigil address
        ldr     r4, =ABORT_SIGIL        @ Load sigil value
        ldr     r3, [r2]                @ Get sigil contents
        cmp     r3, r4                  @ Sigil set?

        strne   r4, [r2]                @ No, store sigil value
        ldrne   r2, =__abort_cnt        @ No, load address of abort counter
        movne   r4, #0                  @ No, Zero for store
        strne   r4, [r2]                @ No, Clear counter

@
@  Now build up structure of registers and stack (r0 = abort address, r1 = 
@  abort type).  This code is based heavily on the work of Roger Lynx, from 
@  http://www.embedded.com/shared/printableArticle.jhtml?articleID=192202641
@
        mrs     r5, cpsr                @ Save current mode to R5 for mode switching
        mrs     r6, spsr                @ spsr_abt = CPSR of dabt originating mode, save to r6 for mode switching
        mov     r2, r6                  @ Building second dataset: r2 = CPSR of exception
        tst     r6, #0x0f               @ Test mode of the raised exception
        orreq   r6, r6, #0x0f           @ If 0, elevate from user mode to system mode
        msr     cpsr_c, r6              @ Switch out from mode 0x17 (abort) to ...
        mov     r3, lr                  @ ... dabt generating mode and state
        mov     r4, sp                  @ ... Get lr (=r3) and sp (=r4)
        msr     cpsr_c, r5              @ Switch back to mode 0x17 (abort)
        cmp     r1, #1                  @ Test for prefetch abort
        moveq   r1, #0                  @ Can't fetch instruction at the abort address
        ldrne   r1, [r0]                @ r1 = [pc] (dabt)
        ldr     sp, =__abort_mem        @ Reset sp to arrays starting address
        stmia   sp, {r0-r4}             @ Save second dataset from r0 to r4

        ldr     r1, =__abort_stk        @ Space where we'll store abort stack
        mov     r2,#8                   @ Copy 8 stack entries
.abtcopy:
        ldr     r0, [r4], #4            @ Get byte from source, r4 += 4
        str     r0, [r1], #4            @ Store byte to destination, r1 += 4
        subs    r2, r2, #1              @ Decrement loop counter
        bgt     .abtcopy                @ >= 0, go again

        b       .sysreset               @ And reset

@
@  Force a system reset with ye olde watch dogge
@
        .set    SCB_RSIR_MASK, 0x0000000f
        .set    SCB_RSIR,      0xe01fc180
        .set    WD_MOD,        0xe0000000
        .set    WD_TC,         0xe0000004
        .set    WD_FEED,       0xe0000008
        .set    WD_MOD_WDEN,   0x00000001
        .set    WD_MOD_RESET,  0x00000002
        .set    WD_MOD_TOF,    0x00000004
        .set    WD_MOD_INT,    0x00000008
        .set    WD_MOD_MASK,   0x0000000f
        .set    WD_FEED_FEED1, 0x000000aa
        .set    WD_FEED_FEED2, 0x00000055
        .set    ABORT_SIGIL,   0xdeadc0de

.sysreset:
        ldr     r1, =__abort_cnt        @ Get the abort counter address
        ldr     r0, [r1]                @ Load it
        add     r0, r0, #1              @ Add 1
        str     r0, [r1]                @ Store it back

@
@  Now enable the watch dog, and go into a loop waiting for a timeout
@
        ldr     r0, =SCB_RSIR_MASK
        ldr     r1, =SCB_RSIR
        str     r0, [r1]
        ldr     r0, =WD_MOD_WDEN | WD_MOD_RESET
        ldr     r1, =WD_MOD
        str     r0, [r1]
        ldr     r0, =120000
        ldr     r1, =WD_TC
        str     r0, [r1]
        ldr     r0, =WD_FEED_FEED1
        ldr     r1, =WD_FEED
        str     r0, [r1]
        ldr     r0, =WD_FEED_FEED2
        ldr     r1, =WD_FEED
        str     r0, [r1]
        b       .

@
@  Reserve space for interrupt vectors in RAM, in case we decide to be tricky.
@  Note that we never actually disable this code if it is not used (i.e.
@  CFG_RAM_INTS not defined).  This is because 'as' is painfully stupid about
@  somethings, and -D parameters on the command line is one of them.  In 
@  theory we could pipe it through 'cpp', but at this point I didn't want to
@  break what appear to be fairly fragile WinARM things.  Use Linux! (Okay,
@  alright, important safety tip, thanks Egon).
@
        .global __intvects
        .section .intvects,"ax"
        .align  0

__intvects:   ldr     pc, _ram_reset          @ reset
              ldr     pc, _ram_undf           @ undefined
              ldr     pc, _ram_swi            @ SWI
              ldr     pc, _ram_pabt           @ program abort
              ldr     pc, _ram_dabt           @ data abort
              ldr     pc, _ram_rsrvd          @ reserved
              ldr     pc, [pc, #-0xff0]       @ IRQ - read VIC_VectAddr (magic!  It's at 0xfffff030)
              ldr     pc, _ram_fiq            @ FIQ

_ram_reset:   .word   __start                 @ reset
_ram_undf:    .word   __undf                  @ undefined                    
_ram_swi:     .word   swiDispatch             @ SWI
_ram_pabt:    .word   __pabt                  @ program abort                
_ram_dabt:    .word   __dabt                  @ data abort                   
_ram_rsrvd:   .word   __start                 @ reserved
_ram_irq:     .word   0                       @ (not used, just reserved space)
_ram_fiq:     .word   fiqISR                  @ FIQ                          

@
@  These are in the .protected space in RAM to make sure that initialization 
@  code doesn't overwrite them.  When a data abort or an undefined instruction
@  exception occurs, the handlers update the respective locations below.  ORDER
@  IS IMPORTANT, THESE ARE MAPPED INTO THE C STRUCTURE abortDat_t in monitor.c
@  (See note above why this is not disabled with CFG_ABORT isn't defined)
@
        .global __abort_dat
        .section .protected
        .align  0

__abort_dat:  .word 0                   @ Dummy, not used
__abort_sig:  .word 0                   @ Sigil to indicate data validity
__abort_cnt:  .word 0                   @ Number of times we've aborted
__abort_typ:  .word 0                   @ Type of abort (0=undef,1=pabort,2=dabort)
__abort_mem:  .space (18 * 4), 0        @ Registers from abort state
__abort_stk:  .space (8 * 4), 0         @ 8 stack entries from abort state

@
@  Define globals so application can figure out what stacks are where.
@  Keep these in order!  The stack setup code expects it.
@
        .global __stack_beg_und
        .global __stack_end_und
        .global __stack_beg_abt
        .global __stack_end_abt
        .global __stack_beg_fiq
        .global __stack_end_fiq
        .global __stack_beg_irq
        .global __stack_end_irq
        .global __stack_beg_svc
        .global __stack_end_svc
        .global __stack_beg_sys
        .global __stack_end_sys
        .global __heap_max
        .global __heap_beg
        .global __heap_end
        .data
        .align  0

__stack_beg_und: .word 0                @ 0
__stack_end_und: .word 0                @ 4
__stack_beg_abt: .word 0                @ 8
__stack_end_abt: .word 0                @ 12
__stack_beg_fiq: .word 0                @ 16
__stack_end_fiq: .word 0                @ 20
__stack_beg_irq: .word 0                @ 24
__stack_end_irq: .word 0                @ 28
__stack_beg_svc: .word 0                @ 32
__stack_end_svc: .word 0                @ 36
__stack_beg_sys: .word 0                @ 40
__stack_end_sys: .word 0                @ 44
__heap_max:      .word 0                @ 48
__heap_beg:      .word __heap_beg__
__heap_end:      .word __heap_end__

        .end
