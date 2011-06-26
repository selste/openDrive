THISDIR		:= openDrive

THISSRCS	:= main.c
THISTARGET	:= openDrive.a

THISOBJS	:= $(THISSRCS:%.c=%.o)
CSRCS		:= $(addprefix $(THISDIR)/,$(THISSRCS))
COBJS		:= $(addprefix $(THISDIR)/,$(THISOBJS))
CTARG		:= $(addprefix $(THISDIR)/,$(THISTARGET))

SRCS		+= $(CSRCS)
OBJS		+= $(COBJS)
TARGETS		+= $(CTARG)

all: $(CTARG)

$(CTARG): $(COBJS) 
	ar cr $@ $+
