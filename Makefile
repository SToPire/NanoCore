BOOTDIR = $(shell pwd)/boot
export KDIR = $(shell pwd)/kernel
export BUILDDIR = $(shell pwd)/build

export CFLAGS = -fPIC -fno-strict-aliasing -O2 -Wall -ggdb -ffreestanding -nostdinc 
CFLAGS += -fno-omit-frame-pointer -fno-stack-protector
CFLAGS += -mno-red-zone -mno-mmx -mno-sse -mno-sse2 

# warning=0, info=1, debug=2
CFLAGS += -DLOG_LEVEL=1

ASFLAGS = -f elf64 -F dwarf -g
LDFLAGS = -m elf_x86_64

QEMU = qemu-system-x86_64 -nographic -serial mon:stdio

KOBJS = $(shell find $(KDIR) -name "*.c" \
| sed -r 's|$(KDIR).*/|$(BUILDDIR)/|g' \
| sed -r 's/.c/.o/g' \
| tr -s "\n" " " )

KSUBDIRS := $(wildcard $(KDIR)/*/.)

all: $(BUILDDIR)/hd.img

$(BUILDDIR)/mbr: $(BOOTDIR)/mbr.S $(BOOTDIR)/mbr.c
	mkdir -p $(BUILDDIR)
	nasm $(ASFLAGS) -i $(BOOTDIR)/include $(BOOTDIR)/mbr.S -o $(BUILDDIR)/mbr.S.o
	gcc $(CFLAGS) -fno-pie -no-pie -O -c $(BOOTDIR)/mbr.c -o $(BUILDDIR)/mbr.c.o 
	ld $(LDFLAGS) -N -e _start -Ttext 0x7c00 -o $(BUILDDIR)/mbr.o $(BUILDDIR)/mbr.S.o $(BUILDDIR)/mbr.c.o
	objcopy -S -O binary -j .text $(BUILDDIR)/mbr.o $@
	dd if=/dev/null of=$@ bs=1 seek=510
# override shell built-in command 'printf'	
	/usr/bin/printf '\x55\xAA' >> $(BUILDDIR)/mbr

$(BUILDDIR)/loader: $(BOOTDIR)/loader.S $(BOOTDIR)/loader.c
	nasm $(ASFLAGS) -i $(BOOTDIR)/include $(BOOTDIR)/loader.S -o $(BUILDDIR)/loader.S.o
	gcc $(CFLAGS) -fno-pie -no-pie -O -c $(BOOTDIR)/loader.c -o $(BUILDDIR)/loader.c.o
	ld $(LDFLAGS) -N -e _start -Ttext 0x8000 -o $(BUILDDIR)/loader.o $(BUILDDIR)/loader.S.o $(BUILDDIR)/loader.c.o
	objcopy -S -O binary -j .text $(BUILDDIR)/loader.o $@

$(BUILDDIR)/kernel: $(KSUBDIRS)
	gcc $(CFLAGS) -I $(KDIR) $(KDIR)/main.c -c -o $(BUILDDIR)/main.o
	ld $(LDFLAGS) -T $(KDIR)/kernel.ld -N -e main -o $@ $(KOBJS)

$(KSUBDIRS): 
	$(MAKE) -f $(KDIR)/Makefile -C $@

$(BUILDDIR)/hd.img: $(BUILDDIR)/mbr $(BUILDDIR)/loader $(BUILDDIR)/kernel
	dd if=$(BUILDDIR)/mbr of=$(BUILDDIR)/hd.img bs=512 count=1 conv=notrunc
	dd if=$(BUILDDIR)/loader of=$(BUILDDIR)/hd.img bs=512 seek=1 conv=notrunc
	dd if=$(BUILDDIR)/kernel of=$(BUILDDIR)/hd.img bs=512 seek=2 conv=notrunc

qemu: $(BUILDDIR)/hd.img
	$(QEMU) -drive file=$(BUILDDIR)/hd.img,format=raw

qemu-gdb: $(BUILDDIR)/hd.img
	$(QEMU) -drive file=$(BUILDDIR)/hd.img,format=raw -S -gdb tcp::6789

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean $(KSUBDIRS)
