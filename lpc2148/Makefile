#
#  $Id: Makefile 364 2009-01-12 22:03:17Z jcw $
#  $Revision: 364 $
#  $Author: jcw $
#  $Date: 2009-01-12 17:03:17 -0500 (Mon, 12 Jan 2009) $
#  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/Makefile $
#

.SILENT: 
.DELETE_ON_ERROR:

#
#  -D CFG_CONSOLE_USB   for console on USB (requires CFG_USB, CFG_USB_SER)
#  -D CFG_CONSOLE_UART0 for console on UART0
#  -D CFG_CONSOLE_UART1 for console on UART1 instead of USB (disables GPS task, baud rate set to 115200)
#  -D CFG_USB           to enable USB support (disables USB mass storage and console if not defined)
#  -D CFG_USB_SER       to enable USB serial support (disabled USB console if not defined)
#  -D CFG_USB_MSC       to enable SD/MMC as a mass storage class device over USB
#  -D CFG_I2C           to enable I2C support (disables AT24C1024 and LM75 if not defined)
#  -D CFG_SPI					  to enable SPI support (disablee M25LC512 and uIP if not defined)
#  -D CFG_RTC           to enable real time clock support
#  -D CFG_IAP           to enable IAP support
#  -D CFG_WDT           to enable watchdog functionality
#  -D CFG_FIQ           to enable fast interrupt request functionality
#  -D CFG_SWI           to enable SWI handling in C (asm code remains, requires CFG_LED)
#  -D CFG_PWM           to enable pulse width modulation support
#  -D CFG_ABORT         to eanble abort decoder and test code
#  -D CFG_RAM_INTS      to enable interrupt vectors in RAM (CFG_FIQ runs faster with this defined)
#  -D CFG_MD            to enable memory dump (needed by CFG_I2C, CFG_AT24C1024, and CFG_M25LC512)
#  -D CFG_MEM           to enable memory toys
#  -D CFG_BEEP					to enable beeper and tune player
#  -D CFG_MISC          to enable port configuration display and type size
#  -D CFG_LED           to enable LED blinking task
#  -D CFG_GPS						to enable parsing GPS data on UART1 (disabled if CFG_CONSOLE_UART1 defined)
#  -D CFG_SENSORS			  to enable sensors task (ADC, DAC)
#  -D CFG_LCD           to enable LCD support (edit lcd/Makefile for 4 vs 8 bit LCDs)
#  -D CFG_KBD           to enable keyboard support (see kbd/README)
#  -D CFG_FATFS         to enable SD/MMC FatFS support
#  -D CFG_UIP           to enable uIP w/ ENC28J60 on SPI0
#  -D CFG_TELNETD       to enable telnet server under uIP
#  -D CFG_HTTPD         to enable http server under uIP
#  -D CFG_SNTP				  to enable sntp client under uIP
#  -D CFG_DHCP          to enable DHCP under uIP
#  -D CFG_AT24C1024     to enable AT24C1024 I2C EEPROM support
#  -D CFG_LM75          to enable LM75 I2C temperature sensor support
#  -D CFG_M25LC512      to enable M25LC512 SPI EEPROM support
#  -D CFG_INTONLY       to remap printf->iprintf, sprintf->siprintf, and snprintf->sniprintf (CFG_GPS disables)
#  -D CFG_AUTO_UIP      to automatically start uIP at reboot (ignored if not CFG_UIP)
#  -D CFG_AUTO_DHCP     to automatically to DHCP for an address at start (ignored if not CFG_UIP, CFG_DHCP and CFG_AUTO_UIP)
#  -D CFG_AUTO_SNTP     to automatically to set the time via SNTP (ignored if not CFG_UIP and CFG_AUTO_UIP)
#
export LPC2148DEMO_OPTS=-D CFG_CONSOLE_USB \
                        -D CFG_USB -D CFG_USB_SER \
                        -D CFG_I2C -D CFG_SPI -D CFG_RTC -D CFG_IAP -D CFG_WDT \
                        -D CFG_FIQ -D CFG_SWI -D CFG_PWM -D CFG_ABORT -D CFG_RAM_INTS \
                        -D CFG_MD -D CFG_MEM -D CFG_BEEP -D CFG_MISC \
                        -D CFG_LED -D CFG_GPS -D CFG_SENSORS -D CFG_LCD -D CFG_KBD \
                        -D CFG_FATFS \
                        -D CFG_UIP -D CFG_HTTPD -D CFG_SNTP -D CFG_DHCP \
                        -D CFG_INTONLY \
                        -D CFG_AUTO_UIP -D CFG_AUTO_DHCP

#
#  These shouldn't need to be changed
#
export CC=arm-unknown-eabi-gcc
export AR=arm-unknown-eabi-ar
export OBJCOPY=arm-unknown-eabi-objcopy
export OBJDUMP=arm-unknown-eabi-objdump
export CRT0=boot.s
export WARNINGS=-Wall -Wextra -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wsign-compare -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused
export CFLAGS=$(WARNINGS) -D RUN_MODE=RUN_FROM_ROM -D GCC_ARM7 $(INCLUDES) $(BASEINCLUDE) -mcpu=arm7tdmi -T$(LDSCRIPT) -g -O3 -fomit-frame-pointer $(LPC2148DEMO_OPTS)
export LDSCRIPT=lpc2148-rom.ld
export LINKER_FLAGS=$(COMMON)/common.a -Xlinker -olpc2148.elf -Xlinker -M -Xlinker -Map=lpc2148.map
export ROOT=$(shell pwd)
export BASEINCLUDE=-I$(ROOT) -I$(ROOT)/FreeRTOS/include
export COMMON=$(ROOT)/common
export STARTEDATTOP=true

#
#  Project sub-directories
#
SUBDIRS=FreeRTOS adc cpu dac eints fatfs fiq gps i2c iap kbd lcd leds mmc monitor newlib pwm rtc sensors spi swi timer uart uip usb usbmass usbser

SRC_FILES = main.c

ARM_OBJ = $(SRC_FILES:.c=.o)

.PHONY: all
all : subdirs lpc2148.hex

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

lpc2148.hex : .depend Makefile lpc2148.elf
	$(OBJCOPY) lpc2148.elf -O ihex lpc2148.hex
	@echo "Length is " `grep __"end_of_text__ = ." *.map | cut -b 17-35` "bytes"

lpc2148.elf : .depend Makefile $(ARM_OBJ) $(COMMON)/common.a $(CRT0) $(LDSCRIPT)
	$(CC) $(CFLAGS) $(ARM_OBJ) -nostartfiles $(CRT0) $(LINKER_FLAGS)
	$(OBJDUMP) -d -S lpc2148.elf >lpc2148.lst

$(ARM_OBJ) : %.o : %.c Makefile .depend
	$(CC) -c $(CFLAGS) $< -o $@

#
#  The .depend files contains the list of header files that the
#  various source files depend on.  By doing this, we'll only
#  rebuild the .o's that are affected by header files changing.
#
.depend:
	$(CC) $(CFLAGS) -M $(SRC_FILES) -o .depend

#
#  Utility targets
#
.PHONY: tags
tags :
	@rm -f tags ctags
	find . -name \*.c -exec ctags -a {} \;
	find . -name \*.h -exec ctags -a {} \;

.PHONY: clean
clean :
	find . -name \*.o -exec rm -f {} \;
	find . -name .depend -exec rm -f {} \;
	rm -f *.map *.lst *.elf *.hex .depend $(COMMON)/common.a sizes.csv tags

.PHONY: svncheck
svncheck :
	find . -name \*.c -exec grep -L HeadURL {} \;
	find . -name \*.h -exec grep -L HeadURL {} \;
	find . -name \*.s -exec grep -L HeadURL {} \;
	find . -name Makefile -exec grep -L HeadURL {} \;

.PHONY: release
release :
	./release.sh

#
#
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
