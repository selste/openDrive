THISDIR		:= FreeRTOS

PORTABLES	:= port.c

THISSRCS	:= croutine.c  list.c  queue.c  tasks.c  timers.c portable/MemMang/heap_sys.c
THISSRCS	+= $(addprefix $(PORTABLE_DIR)/,$(PORTABLES))
THISTARGET	:= FreeRTOS.a

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
