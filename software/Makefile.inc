SHELL=/bin/bash
CC=gcc

CFLAGS   = -g -Wno-switch-enum
INCLUDE  = -I./include
DEPDIR   = .deps

DEPDIRS := $(addprefix $(DEPDIR)/,$(SUBDIRS))
SRCS    := $(foreach sdir,$(SUBDIRS),$(wildcard $(sdir)/*.c))
OBJS	:= $(SRCS:%.c=%.o)

DSUBDIR = $(DEPDIR)/$(*D)
df = $(DEPDIR)/$(*D)/$(*F)
IDIR    = ./include/$(*D)

%.exist :
	mkdir -p $@

test:
	@echo DEPDIRS = $(DEPDIRS)
	@echo SRCS = $(SRCS)
	@echo OBJS = $(OBJS)

clean:
	@echo -n Cleaning up OBJS
	@rm $(OBJS) 2>/dev/null || true
	@echo ...[Done]
	@echo -n Cleaning up DEPDIRS 
	@rm -fr $(DEPDIRS) 2>/dev/null || true
	@echo ...[Done]

$(DEPDIR)/%.P : %.c
	@[ -d $(DSUBDIR) ] || mkdir -p $(DSUBDIR)
	@$(CPP) $(INCLUDE) -I$(IDIR) $(CPPFLAGS) $< \
	 | sed -n 's+^\# *[0-9][0-9]* *"\([^"]*\)".*+$*.o: \1+p' \
	 | grep -v "<" | sort | uniq \
	 | sed 's+\($*\)\.o[ :]*+\1.o $@ : +g' > $@; \
         [ -s $@ ] || rm -f $@

%.o : %.c
	@[ -d $(DSUBDIR) ] || mkdir -p $(DSUBDIR)
	@echo -n Compiling $@
	-@$(COMPILE.c) $(INCLUDE) -I$(IDIR) -Wp,-MMD,$(df).d -o $@ $< \
	 2>$(df).err
	@cp $(df).d $(df).P; \
         sed -e 's+#.*++' -e 's+^[^:]*: *++' -e 's+ *\\$$++' \
            -e '/^$$/ d' -e 's+$$+ :+' < $(df).d >> $(df).P; \
         rm -f $(df).d
	@[ ! -s $(df).err ] || ( echo "" && cat $(df).err && echo "" && false )
	@rm $(df).err
	@echo ...[Done]

-include $(SRCS:%.c=$(DEPDIR)/%.P)
