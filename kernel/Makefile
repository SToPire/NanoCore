# This Makefile is used in each subdir of kernel/

OBJS := $(patsubst %.c,%.o,$(wildcard *.c))

all: $(OBJS)

%.o: %.c
	gcc $(CFLAGS) -I $(KDIR) $< -c -o $@
	mv $@ $(BUILDDIR)/