THISDIR		:= novas

THISSRCS	:= eph_manager.c novas.c novascon.c nutation.c readeph0.c \
		   solsys1.c

THISOBJS	:= $(THISSRCS:%.c=%.o)
CSRCS		:= $(addprefix $(THISDIR)/,$(THISSRCS))
COBJS		:= $(addprefix $(THISDIR)/,$(THISOBJS))

SRCS		+= $(CSRCS)
OBJS		+= $(COBJS)

