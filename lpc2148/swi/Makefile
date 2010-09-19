#
#  $Id: Makefile 338 2008-11-10 04:15:26Z jcw $
#  $Revision: 338 $
#  $Author: jcw $
#  $Date: 2008-11-09 23:15:26 -0500 (Sun, 09 Nov 2008) $
#  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/swi/Makefile $
#

SRC_FILES=swi.c
ASMSRC_FILES=swidispatch.s

#
# Define all object files.
#
ARM_OBJ = $(SRC_FILES:.c=.o)
ARM_ASMOBJ = $(ASMSRC_FILES:.s=.o)

.PHONY: all
ifeq ($(STARTEDATTOP),true)
all: $(ARM_OBJ) $(ARM_ASMOBJ)
else
all :
	@echo "Project must be rebuilt from top level"
endif

$(ARM_OBJ) : %.o : %.c Makefile .depend
	$(CC) -c $(CFLAGS) $< -o $@
	$(AR) rc $(COMMON)/common.a $@

$(ARM_ASMOBJ) : %.o : %.s Makefile .depend
	$(CC) -c $(CFLAGS) $< -o $@
	$(AR) rc $(COMMON)/common.a $@

#
#  The .depend files contains the list of header files that the
#  various source files depend on.  By doing this, we'll only
#  rebuild the .o's that are affected by header files changing.
#
.depend:
	$(CC) $(CFLAGS) -M $(SRC_FILES) -o .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
