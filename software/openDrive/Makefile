THISDIR		:= openDrive

THISSRCS	:= main.c hw.c uart.c pwm.c cmdparser.c debug.c led.c \
		   motor_control.c \
                   TMC428.c

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
