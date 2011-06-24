SUBDIRS = novas FreeRTOS  openDrive CMSIS/CM3/CoreSupport \
	CMSIS/CM3/DeviceSupport/ARM CMSIS/CM3/DeviceSupport/LPC17xx
LIBS	= libnovas.a
LNOBJS = eph_manager.o novas.o novascon.o nutation.o readeph0.o solsys1.o solsys2.o solsys3.o

LNOBJS2 := $(addprefix novas/,$(LNOBJS))


default: all
	echo Making $@ from $+

include Makefile.inc

all: novas/libnovas.a $(OBJS)

novas/libnovas.a: $(LNOBJS2) 
	ar cr $@ $+

