#
#       !!!! Do NOT edit this makefile with an editor which replace tabs by spaces !!!!    
#
##############################################################################################
# 
# On command line:
#
# make all = Create project
#
# make clean = Clean project files.
#
# To rebuild project do "make clean" and "make all".
#

##############################################################################################
# Start of default section
#

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
GDB  = $(TRGT)gdb
BIN  = $(CP) -O ihex 

MCU  = cortex-m3

# List all default C defines here, like -D_DEBUG=1
DDEFS = 

# List all default ASM defines here, like -D_DEBUG=1
DADEFS = 

# List all default directories to look for include files here
DINCDIR = 

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = 

#
# End of default section
##############################################################################################

##############################################################################################
# Start of user section
#

# Define project name here
PROJECT = test

# Define linker script file here
LDSCRIPT_FLASH = ./sys/rtosdemo_Debug.ld

# List all user C define here, like -D_DEBUG=1
UDEFS = 

# Define ASM defines here
UADEFS = 

# List C source files here
SRC  = ./sys/startup_lpc17.c \
       ./sys/syscalls.c \
       ../../FreeRTOS/list.c \
       ../../FreeRTOS/queue.c \
       ../../FreeRTOS/tasks.c \
       ../../FreeRTOS/portable/GCC/ARM_CM3/port.c \
       ../../FreeRTOS/portable/MemMang/heap_2.c \
       ../../CMSIS/CM3/DeviceSupport/LPC17xx/system_LPC17xx.c \
       ./debug.c \
       ./main.c \
       ./led_port.c \
       ./uart_port.c \
       ./spi_port.c \
       ../../openDrive/led.c \
       ../../openDrive/uart.c \
       ../../openDrive/test.c \
       ../../openDrive/spi.c
             

# List ASM source files here
ASRC = 

# List all user directories here
UINCDIR = ./include \
          D:/Yagarto/arm-none-eabi/include \
          D:/Yagarto/arm-none-eabi/sys-include \
          ../../CMSIS/CM3/CoreSupport \
          ../../CMSIS/CM3/DeviceSupport/LPC17xx \
          ../../FreeRTOS/include \
          ../../FreeRTOS/portable/GCC/ARM_CM3 \
          ../../openDrive          

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = 

# Define optimisation level here
OPT = -O0

#
# End of user defines
##############################################################################################


INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))
DEFS    = $(DDEFS) $(UDEFS)
ADEFS   = $(DADEFS) $(UADEFS)
OBJS    = $(ASRC:.s=.o) $(SRC:.c=.o)
LIBS    = $(DLIBS) $(ULIBS)
MCFLAGS = -mcpu=$(MCU)

ASFLAGS = $(MCFLAGS) -g2 -gdwarf-2 -mthumb -Wa,-amhls=$(<:.s=.lst) $(ADEFS)
CPFLAGS = $(MCFLAGS) $(OPT) -gdwarf-2 -mthumb -fomit-frame-pointer -Wall -Wstrict-prototypes -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DEFS)
#ASFLAGS = $(MCFLAGS) -g2 -gstabs -mthumb -Wa,-amhls=$(<:.s=.lst) $(ADEFS)
#CPFLAGS = $(MCFLAGS) $(OPT) -g2 -gstabs -mthumb -fomit-frame-pointer -Wall -Wstrict-prototypes -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DEFS)
LDFLAGS_FLASH = $(MCFLAGS) -mthumb -nostartfiles -T$(LDSCRIPT_FLASH) -Wl,-Map=$(PROJECT)_flash.map,--cref,--no-warn-mismatch $(LIBDIR)

# Generate dependency information
CPFLAGS += -MD -MP -MF .dep/$(@F).d

#
# makefile rules
#

all: FLASH

FLASH: $(OBJS) $(PROJECT)_flash.elf $(PROJECT)_flash.hex

%o : %c
	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@

%o : %s
	$(AS) -c $(ASFLAGS) -I . $(INCDIR) $< -o $@

%flash.elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS_FLASH) $(LIBS) -o $@

%hex: %elf
	$(BIN) $< $@
	

# Program the device.  
#program:
	#arm-none-eabi-gdb --batch -x sys/gdbflashdownload.jlink
	#@$(GDB) --batch -x "sys/gdbflashdownload.jlink"


clean:
	-rm -f $(OBJS)
	-rm -f $(PROJECT)_flash.elf
	-rm -f $(PROJECT)_flash.map
	-rm -f $(PROJECT)_flash.hex
	-rm -f $(SRC:.c=.c.bak)
	-rm -f $(SRC:.c=.lst)
	-rm -f $(ASRC:.s=.s.bak)
	-rm -f $(ASRC:.s=.lst)
	-rm -fR .dep

# 
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***