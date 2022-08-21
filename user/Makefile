UPROG = $(shell find $(UDIR) -maxdepth 1 -name "*.c" \
| sed -r 's|$(UDIR)/|u_|g' \
| sed -r 's/.c//g' \
| tr -s "\n" " " )

tarfs: $(UPROG)
# remove 'u_' prefix 
	for f in `ls $(BUILDDIR)/user/bin`; do \
		mv "$(BUILDDIR)/user/bin/$$f" "$(BUILDDIR)/user/bin/`echo $$f | sed 's/u_//g'`" ; \
	done
#	generate tar archive	
	tar cf $(BUILDDIR)/user/tarfs.img -C $(BUILDDIR)/user/bin/ `ls $(BUILDDIR)/user/bin`
# align tarfs.img to 16 sectors
	dd if=/dev/null of=$(BUILDDIR)/user/tarfs.img bs=512 seek=16

u_%: %.c
	gcc $(CFLAGS) $< -c -o $(BUILDDIR)/user/$@.o
	ld $(LDFLAGS) -T $(UDIR)/user.ld -e main $(BUILDDIR)/user/$@.o -o $(BUILDDIR)/user/bin/$@