#******************************************************************************
#
# Makefile - Rules for building the opendrive example.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
# NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
# NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
# CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
# DAMAGES, FOR ANY REASON WHATSOEVER.
#
#******************************************************************************

#
# Defines the part type that this project uses.
#
PART=LM3S6965

#
# The base directory for StellarisWare.
#
ROOT=../../..

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH=../drivers

#
# Where to find header files that do not live in the source directory.
#
IPATH=..
IPATH+=../../..

#
# The default rule, which causes the hello example to be built.
#
all: ${COMPILER}
all: ${COMPILER}/opendrive.axf

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} ${wildcard *~}

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir ${COMPILER}

#
# Rules for building the hello example.
#
${COMPILER}/opendrive.axf: ${COMPILER}/opendrive.o
${COMPILER}/opendrive.axf: ${COMPILER}/rit128x96x4.o
${COMPILER}/opendrive.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/opendrive.axf: ${ROOT}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/opendrive.axf: opendrive.ld
SCATTERgcc_opendrive=opendrive.ld
ENTRY_opendrive=ResetISR

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif


flash: all
	openocd -f init.cfg -c "flash write_bank 0 gcc/opendrive.bin 0x0" -c "sleep 200" -c "reset run" -c "shutdown"
