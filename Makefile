BOOTDIR = $(shell pwd)/boot
export KDIR = $(shell pwd)/kernel
export BUILDDIR = $(shell pwd)/build
export UDIR = $(shell pwd)/user

export CFLAGS = -fPIC -fno-strict-aliasing -O2 -Wall -ggdb -ffreestanding -nostdinc 
CFLAGS += -fno-omit-frame-pointer -fno-stack-protector
CFLAGS += -mno-red-zone -mno-mmx -mno-sse -mno-sse2 

# warning=0, info=1, debug=2
CFLAGS += -DLOG_LEVEL=2

export ASFLAGS = -f elf64 -F dwarf -g
export LDFLAGS = -m elf_x86_64 --no-relax

QEMU = qemu-system-x86_64 -nographic -serial mon:stdio -m 512 \
				-drive file=$(BUILDDIR)/hd.img,format=raw \
				-drive file=$(BUILDDIR)/user/tarfs.img,format=raw \

KCOBJS = $(shell find $(KDIR) -name "*.c" \
| sed -r 's|$(KDIR).*/|$(BUILDDIR)/|g' \
| sed -r 's/\.c/.o/g' \
| tr -s "\n" " " )

KASMOBJS = $(shell find $(KDIR) -name "*.S" \
| sed -r 's|$(KDIR).*/|$(BUILDDIR)/|g' \
| sed -r 's/\.S/.S.o/g' \
| tr -s "\n" " " )

KSUBDIRS := $(wildcard $(KDIR)/*/.)

build:
	docker run -it --rm -u $(shell id -u ${USER}):$(shell id -g ${USER}) -v $(shell pwd):/sos -w /sos stopire/sos-builder:v1 make docker-build

docker-build: kernel user

$(BUILDDIR)/mbr: $(BOOTDIR)/mbr.S $(BOOTDIR)/mbr.c
	mkdir -p $(BUILDDIR)
	nasm $(ASFLAGS) -i $(BOOTDIR)/include $(BOOTDIR)/mbr.S -o $(BUILDDIR)/mbr.S.o
	gcc $(CFLAGS) -fno-pie -no-pie -O -c $(BOOTDIR)/mbr.c -o $(BUILDDIR)/mbr.c.o 
	ld $(LDFLAGS) -N -e _start -Ttext 0x7c00 -o $(BUILDDIR)/mbr.o $(BUILDDIR)/mbr.S.o $(BUILDDIR)/mbr.c.o
	objcopy -S -O binary -j .text $(BUILDDIR)/mbr.o $@
	dd if=/dev/null of=$@ bs=1 seek=510
# override shell built-in command 'printf'	
	/usr/bin/printf '\x55\xAA' >> $@

$(BUILDDIR)/loader: $(BOOTDIR)/loader.S $(BOOTDIR)/loader.c
	nasm $(ASFLAGS) -i $(BOOTDIR)/include $(BOOTDIR)/loader.S -o $(BUILDDIR)/loader.S.o
	gcc $(CFLAGS) -fno-pie -no-pie -O -c $(BOOTDIR)/loader.c -o $(BUILDDIR)/loader.c.o
	ld $(LDFLAGS) -N -e _start -Ttext 0x8000 -o $(BUILDDIR)/loader.o $(BUILDDIR)/loader.S.o $(BUILDDIR)/loader.c.o
	objcopy -S -O binary -j .text $(BUILDDIR)/loader.o $@

$(BUILDDIR)/kernel: $(KSUBDIRS)
	nasm $(ASFLAGS) $(KDIR)/entry.S -o $(BUILDDIR)/entry.S.o
	gcc $(CFLAGS) -I $(KDIR)/include $(KDIR)/main.c -c -o $(BUILDDIR)/main.o
	ld $(LDFLAGS) -T $(KDIR)/kernel.ld -N -e kentry -o $@ $(KCOBJS) $(KASMOBJS)

$(KSUBDIRS): 
	$(MAKE) -f $(KDIR)/Makefile -C $@

bootloader: $(BUILDDIR)/mbr $(BUILDDIR)/loader

kernel: bootloader $(BUILDDIR)/kernel
	dd if=$(BUILDDIR)/mbr of=$(BUILDDIR)/hd.img bs=512 count=1 conv=notrunc
	dd if=$(BUILDDIR)/loader of=$(BUILDDIR)/hd.img bs=512 seek=1 conv=notrunc
	dd if=$(BUILDDIR)/kernel of=$(BUILDDIR)/hd.img bs=512 seek=2 conv=notrunc

user:
	mkdir -p $(BUILDDIR)/user/bin
	$(MAKE) -f $(UDIR)/Makefile -C $(UDIR)

qemu: 
	$(QEMU)

qemu-gdb: 
	$(QEMU) -S -gdb tcp::6789

gen-builder:
	docker build -t sos-builder --network=host .

compile-commands:
# compile locally, correctness is not important
	bear -- $(MAKE) docker-build
	rm -rf build

clean:
	rm -rf $(BUILDDIR)

.PHONY: build clean gen-builder user $(KSUBDIRS)
