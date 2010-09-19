#
#  $Id: Makefile 259 2008-10-31 01:03:38Z jcw $
#  $Revision: 259 $
#  $Author: jcw $
#  $Date: 2008-10-30 21:03:38 -0400 (Thu, 30 Oct 2008) $
#  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/FreeRTOS/portable/GCC/Makefile $
#

#
# Define all object files.
#
SUBDIRS=ARM7_LPC2000

.PHONY: all
ifeq ($(STARTEDATTOP),true)
all : subdirs
else
all :
	@echo "Project must be rebuilt from top level"
endif

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
