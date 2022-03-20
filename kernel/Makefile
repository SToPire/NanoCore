# This Makefile is used in each subdir of kernel/

COBJS := $(patsubst %.c,%.o,$(wildcard *.c))
ASMOBJS := $(patsubst %.S,%.S.o,$(wildcard *.S))

all: $(COBJS) $(ASMOBJS)

%.S.o: %.S
	nasm $(ASFLAGS) $< -o $@
	mv $@ $(BUILDDIR)/

%.o: %.c
	gcc $(CFLAGS) -I $(KDIR)/include $< -c -o $@
	mv $@ $(BUILDDIR)/