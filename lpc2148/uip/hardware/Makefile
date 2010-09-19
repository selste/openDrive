#
#  $Id: Makefile 338 2008-11-10 04:15:26Z jcw $
#  $Revision: 338 $
#  $Author: jcw $
#  $Date: 2008-11-09 23:15:26 -0500 (Sun, 09 Nov 2008) $
#  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uip/hardware/Makefile $
#

SRC_FILES=enc28j60.c

#
# Define all object files.
#
ARM_OBJ = $(SRC_FILES:.c=.o)

.PHONY: all
ifeq ($(STARTEDATTOP),true)
all: $(ARM_OBJ)
else
all :
	@echo "Project must be rebuilt from top level"
endif

$(ARM_OBJ) : %.o : %.c Makefile .depend
	$(CC) -c $(CFLAGS) -Wno-cast-align $< -o $@
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
