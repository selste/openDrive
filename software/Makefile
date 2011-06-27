PORT		:= GCC
PLATFORM	:= LPC1768
#LIBS		:= -lpthread
LIBS		:= 

SUBDIRS 	+= novas openDrive FreeRTOS

# -----------------   S   N   I   P -----------------
# --- No need to change anything below this line. ---
OBJECT		:= openDrive.bin
SHELL		:=/bin/bash
DEFINES		:= -DPORT=$(PORT) -DPLATFORM=$(PLATFORM)
CC		:= gcc
CFLAGS		+= -g -Wno-unused-result

PORTABLE_DIR	:= portable/$(PORT)/$(PLATFORM)
INCLUDES	+= -I. -I./include -I./include/$(PORTABLE_DIR)
INCLUDE_THIS	:= -I./include/$(*D) # hmpf this does not expand

DEPDIR   	:= .deps
DSUBDIR 	:= $(DEPDIR)/$(*D)
df 		:= $(DEPDIR)/$(*D)/$(*F)

SRCS		:=
OBJS		:=
TARGETS		:=

MAKEFILES	:= $(foreach sdir,$(SUBDIRS),$(sdir)/Makefile)
-include $(MAKEFILES)

DEPDIRS 	:= $(addprefix $(DEPDIR)/,$(SUBDIRS))
DEPENDENCIES	:= $(OBJS):%.o=$(DEPDIR)/%.P)

-include $(SRCS:%.c=$(DEPDIR)/%.P)

all : $(OBJECT)

$(OBJECT): $(TARGETS)
	gcc -static $+ -o $@ $(LIBS)

test:
	@echo Makefiles = $(MAKEFILES)
	@echo DEPDIRS   = $(DEPDIRS)
	@echo SRCS      = $(SRCS)
	@echo OBJS      = $(OBJS)
	@echo TAGRETS   = $(TARGETS)

rebuild:
	touch $(SRCS)

clean:
	@echo -n Cleaning up DEPDIRS 
	@rm -fr $(DEPDIR) 2>/dev/null || true
	@echo ...[Done]
	@echo -n Cleaning up TARGETS
	@rm $(TARGETS) 2>/dev/null || true
	@echo ...[Done]
	@echo -n Cleaning up OBJS
	@rm $(OBJECT) $(OBJS) 2>/dev/null || true
	@echo ...[Done]

$(DEPDIR)/%.P : %.c
	@[ -d `dirname $@` ] || mkdir -p `dirname $@` 
	$(CPP) $(INCLUDES) $(DEFINES) \
	       -I./include/$(*D) -I./include/$(PORTABLE_DIR)/$(*D) \
		$(CPPFLAGS) $< \
	 | sed -n 's+^\# *[0-9][0-9]* *"\([^"]*\)".*+$*.o: \1+p' \
	 | grep -v "<" | sort | uniq \
	 | sed 's+\($*\)\.o[ :]*+\1.o $@ : +g' > $@; \
        [ -s $@ ] || rm -f $@

%.o : %.c
	@[ -d $(DSUBDIR) ] || mkdir -p $(DSUBDIR)
	@echo -n Compiling $@
	-$(COMPILE.c) $(INCLUDES) $(DEFINES) \
		       -I./include/$(*D) -I./include/$(PORTABLE_DIR)/$(*D) \
	               -Wp,-MMD,$(df).d \
	               -o $@ $< 2>$(df).err
	@cp $(df).d $(df).P; \
         sed -e 's+#.*++' -e 's+^[^:]*: *++' -e 's+ *\\$$++' \
             -e '/^$$/ d' -e 's+$$+ :+' < $(df).d >> $(df).P; \
         rm -f $(df).d
	@[ ! -s $(df).err ] || ( echo "" && cat $(df).err && echo "" && false )
	@rm $(df).err
	@echo ...[Done]
