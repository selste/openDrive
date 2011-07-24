PORT		:= Intel
PLATFORM	:= Linux
LIBS		:= -lpthread
CROSS_COMPILE   := 

PORT		:= GCC
PLATFORM	:= LPC1768
LIBS		:=
CROSS_COMPILE   := arm-elf-

SUBDIRS 	+= novas openDrive FreeRTOS

# -----------------   S   N   I   P -----------------
# --- No need to change anything below this line. ---
TARGET		:= openDrive
FORMAT 		:= ihex
SHELL		:= /bin/bash
DEFINES		:= -DPORT=$(PORT) -DPLATFORM=$(PLATFORM)
LIBS		+= -lm
CC		:= $(CROSS_COMPILE)gcc
CFLAGS		+= -g 
AR		:= $(CROSS_COMPILE)ar 
OBJCOPY		:= $(CROSS_COMPILE)objcopy
OBJDUMP		:= $(CROSS_COMPILE)objdump
NM		:= $(CROSS_COMPILE)nm

PORTABLE_DIR	:= portable/$(PORT)/$(PLATFORM)
INCLUDES	+= -I. -I./include -I./include/$(PORTABLE_DIR)
INCLUDE_THIS	:= -I./include/$(*D) # hmpf this does not expand

DEPDIR   	:= .deps
DSUBDIR 	:= $(DEPDIR)/$(*D)
df 		:= $(DEPDIR)/$(*D)/$(*F)

SRCS		:=
OBJS		:=

MAKEFILES	:= $(foreach sdir,$(SUBDIRS),$(sdir)/Makefile)
-include $(MAKEFILES)

DEPDIRS 	:= $(addprefix $(DEPDIR)/,$(SUBDIRS))
DEPENDENCIES	:= $(OBJS):%.o=$(DEPDIR)/%.P)


all : build

-include $(SRCS:%.c=$(DEPDIR)/%.P)

test:
	@echo Makefiles = $(MAKEFILES)
	@echo DEPDIRS   = $(DEPDIRS)
	@echo SRCS      = $(SRCS)
	@echo OBJS      = $(OBJS)

rebuild:
	touch $(SRCS)

clean:
	@echo -n Cleaning up DEPDIRS 
	@rm -fr $(DEPDIR) 2>/dev/null || true
	@echo ...[Done]
	@echo -n Cleaning up OBJS
	@rm $(OBJECT) $(OBJS) 2>/dev/null || true
	@echo ...[Done]
	@echo -n Cleaning up TARGETS
	@rm $(TARGET).* 2>/dev/null || true
	@echo ...[Done]

ifeq ($(FORMAT),ihex)
build: elf hex lss sym
hex: $(TARGET).hex
IMGEXT=hex
else 
ifeq ($(FORMAT),binary)
build: elf bin lss sym
bin: $(TARGET).bin
IMGEXT=bin
else 
$(error "$(MSG_FORMATERROR) $(FORMAT)")
endif
endif

elf: $(TARGET).elf
lss: $(TARGET).lss 
sym: $(TARGET).sym

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
	@echo -n Creating $(*F).hex
	@$(OBJCOPY) -O $(FORMAT) $< $@
	@echo ...[Done]

# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@echo -n Creating $(*F).bin
	@$(OBJCOPY) -O $(FORMAT) $< $@
	@echo ...[Done]

# Create extended listing file from ELF output file.
# testing: option -C
%.lss: %.elf
	@echo -n Creating $(*F).lss
	@$(OBJDUMP) -h -S -C $< > $@
	@echo ...[Done]

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo -n Creating $(*F).sym
	@$(NM) -n $< > $@
	@echo ...[Done]

# Link: create ELF output file from object files.
%.elf:  $(OBJS)
	@echo -n Creating $(*F).elf
	@$(CC) $(THUMB) $+ --output $@ $(LIBS) $(LDFLAGS)
	@echo ...[Done]

$(DEPDIR)/%.P : %.c
	@[ -d `dirname $@` ] || mkdir -p `dirname $@` 
	@$(CPP) $(INCLUDES) $(DEFINES) \
	       -I./include/$(*D) -I./include/$(PORTABLE_DIR)/$(*D) \
		$(CPPFLAGS) $< \
	 | sed -n 's+^\# *[0-9][0-9]* *"\([^"]*\)".*+$*.o: \1+p' \
	 | grep -v "<" | sort | uniq \
	 | sed 's+\($*\)\.o[ :]*+\1.o $@ : +g' > $@; \
         [ -s $@ ] || rm -f $@

%.o : %.c
	@[ -d $(DSUBDIR) ] || mkdir -p $(DSUBDIR)
	@echo -n Compiling $@
	@$(COMPILE.c) $(INCLUDES) $(DEFINES) \
		       -I./include/$(*D) -I./include/$(PORTABLE_DIR)/$(*D) \
	               -Wp,-MMD,$(DEPDIR)/$(*D)/$(*F).d \
	               -o $@ $< 2>$(DEPDIR)/$(*D)/$(*F).err; \
	 cp $(DEPDIR)/$(*D)/$(*F).d $(DEPDIR)/$(*D)/$(*F).P; \
         sed -e 's+#.*++' -e 's+^[^:]*: *++' -e 's+ *\\$$++' \
             -e '/^$$/ d' -e 's+$$+ :+' < $(DEPDIR)/$(*D)/$(*F).d >> $(DEPDIR)/$(*D)/$(*F).P; \
         rm -f $(DEPDIR)/$(*D)/$(*F).d || echo ...[Done]
	@[ ! -s $(DEPDIR)/$(*D)/$(*F).err ] || ( echo "" && ! cat $(DEPDIR)/$(*D)/$(*F).err  )
	@rm $(DEPDIR)/$(*D)/$(*F).err
	@echo ...[Done]

% : %.c
	@[ -d $(DSUBDIR) ] || mkdir -p $(DSUBDIR)
	@echo -n Compiling $@
	@$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -DTEST_MODE \
		       -I./include/$(*D) -I./include/$(PORTABLE_DIR)/$(*D) \
	               -o $@ $< 2>$(DEPDIR)/$(*D)/$(*F).err
	@[ ! -s $(DEPDIR)/$(*D)/$(*F).err ] || ( echo "" && ! cat $(DEPDIR)/$(*D)/$(*F).err  )
	@rm $(DEPDIR)/$(*D)/$(*F).err
	@echo ...[Done]
