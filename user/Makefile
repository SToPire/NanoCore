UPROG = $(shell find $(UDIR) -maxdepth 1 -name "*.c" \
| sed -r 's|$(UDIR)/|u_|g' \
| sed -r 's/.c//g' \
| tr -s "\n" " " )

tarfs: $(UPROG)
# remove 'u_' prefix
	for f in `ls $(BUILDDIR)/user/bin`; do \
		mv "$(BUILDDIR)/user/bin/$$f" "$(BUILDDIR)/user/bin/`echo $$f | sed 's/u_//g'`" ; \
	done
# generate tar archive
	tar cf $(BUILDDIR)/user/tarfs.img -C $(BUILDDIR)/user/bin/ `ls $(BUILDDIR)/user/bin`
# align tarfs.img to 128 sectors
	dd if=/dev/null of=$(BUILDDIR)/user/tarfs.img bs=512 seek=128

userlib: $(UDIR)/lib/usyscall.S $(UDIR)/lib/userlib.c
	nasm $(ASFLAGS) $(UDIR)/lib/usyscall.S -o $(BUILDDIR)/user/usyscall.o
	gcc $(CFLAGS) $(UDIR)/lib/userlib.c -c -o $(BUILDDIR)/user/userlib.o

u_%: %.c userlib
	gcc $(CFLAGS) $< -c -o $(BUILDDIR)/user/$@.o
	ld $(LDFLAGS) -T $(UDIR)/user.ld -e main $(BUILDDIR)/user/$@.o $(BUILDDIR)/user/usyscall.o $(BUILDDIR)/user/userlib.o -o $(BUILDDIR)/user/bin/$@