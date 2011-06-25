THISDIR		:= novas

THISSRCS	:= eph_manager.c novas.c novascon.c nutation.c readeph0.c \
		   solsys1.c solsys2.c solsys3.c

THISTARGET	:= libnovas.a

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