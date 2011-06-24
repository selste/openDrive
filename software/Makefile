SUBDIRS = novas FreeRTOS

LIBS	= libnovas.a
LNOBJS = eph_manager.o novas.o novascon.o nutation.o readeph0.o solsys1.o solsys2.o solsys3.o

LNOBJS2 := $(addprefix novas/,$(LNOBJS))

novas/libnovas.a: $(LNOBJS2)
	ar cr $@ $+

default: novas/libnovas.a

include Makefile.inc

