BOOTDIR = boot
KDIR = kernel
BUILDDIR = build

# CFLAGS from xv6
CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer
CFLAGS += $(shell gcc -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
# Disable PIE when possible (for Ubuntu 16.10 toolchain)
ifneq ($(shell gcc -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell gcc -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif

ASFLAGS = -f elf32 -F dwarf -g
LDFLAGS = -m elf_i386

QEMU = qemu-system-i386 -nographic 

kobj = $(shell find $(KDIR) -name "*.c" \
| tr -s "\n" " " | sed -r 's/.c/.o/g' \
| sed -r 's/$(KDIR)/$(BUILDDIR)/g')

all: $(BUILDDIR)/hd.img

$(BUILDDIR)/mbr: $(BOOTDIR)/boot.S $(BOOTDIR)/loader.c
	mkdir -p $(BUILDDIR)
	nasm $(ASFLAGS) $(BOOTDIR)/boot.S -o $(BUILDDIR)/boot.o
	gcc $(CFLAGS) -O -nostdinc -c $(BOOTDIR)/loader.c -o $(BUILDDIR)/loader.o 
	ld $(LDFLAGS) -N -e _start -Ttext 0x7c00 -o $(BUILDDIR)/mbr.o $(BUILDDIR)/boot.o $(BUILDDIR)/loader.o
	objcopy -S -O binary -j .text $(BUILDDIR)/mbr.o $(BUILDDIR)/mbr
	dd if=/dev/null of=$(BUILDDIR)/mbr bs=1 seek=510
# override shell built-in command 'printf'	
	/usr/bin/printf '\x55\xAA' >> $(BUILDDIR)/mbr

$(BUILDDIR)/kernel: $(kobj)
	ld $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^

$(BUILDDIR)/%.o : $(KDIR)/%.c
	gcc $(CFLAGS) $< -c -o $@ 

$(BUILDDIR)/hd.img: $(BUILDDIR)/mbr $(BUILDDIR)/kernel
	dd if=$(BUILDDIR)/mbr of=$(BUILDDIR)/hd.img bs=512 count=1 conv=notrunc
	dd if=$(BUILDDIR)/kernel of=$(BUILDDIR)/hd.img bs=512 seek=1 conv=notrunc

qemu: $(BUILDDIR)/hd.img
	$(QEMU) -drive file=$(BUILDDIR)/hd.img,format=raw

qemu-gdb: $(BUILDDIR)/hd.img
	$(QEMU) -drive file=$(BUILDDIR)/hd.img,format=raw -S -gdb tcp::6789

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
