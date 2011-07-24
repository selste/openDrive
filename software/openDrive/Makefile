THISDIR		:= openDrive

THISSRCS	:= main.c hw.c uart.c pwm.c cmdparser.c debug.c led.c \
		   motor_control.c \
                   TMC428.c

THISOBJS	:= $(THISSRCS:%.c=%.o)
CSRCS		:= $(addprefix $(THISDIR)/,$(THISSRCS))
COBJS		:= $(addprefix $(THISDIR)/,$(THISOBJS))

SRCS		+= $(CSRCS)
OBJS		+= $(COBJS)

