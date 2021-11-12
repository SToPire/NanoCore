BOOTDIR = boot
BUILDDIR = build

AS = $(shell which nasm)

QEMU = qemu-system-i386 -nographic

all: $(BUILDDIR)/hd.img

$(BUILDDIR)/mbr: $(BOOTDIR)/mbr.S
	mkdir -p $(BUILDDIR)
	$(AS) $(BOOTDIR)/mbr.S -o $(BUILDDIR)/mbr

$(BUILDDIR)/hd.img: $(BUILDDIR)/mbr
	dd if=$(BUILDDIR)/mbr of=$(BUILDDIR)/hd.img bs=512 count=1 conv=notrunc

qemu: $(BUILDDIR)/hd.img
	$(QEMU) -drive file=$(BUILDDIR)/hd.img,format=raw

qemu-gdb: $(BUILDDIR)/hd.img
	$(QEMU) -drive file=$(BUILDDIR)/hd.img,format=raw -S -gdb tcp::6789

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
