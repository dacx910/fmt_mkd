CC=cc
CFLAGS=-Wall -nolibc -nostdlib -nostartfiles -fno-unwind-tables -fno-asynchronous-unwind-tables -g -static -fno-stack-protector -ffreestanding -fno-ident

.PHONY: clean

fmt_mkd: main.c build/std.o build/string.o
	${CC} ${CFLAGS} $^ -o $@
	objcopy -R .comment -R .note.gnu.property -R .note.gnu.build-id $@ $@
build/std.o: include/std.c include/std.h include/crt.h include/lib.h include/linux.h include/types.h include/x64.h
	${CC} ${CFLAGS} -c include/std.c -o $@
build/string.o: include/string.h include/string.c
	${CC} ${CFLAGS} -Wno-discarded-qualifiers -c include/string.c -o $@

clean:
	rm -f fmt_mkd build/std.o build/string.o
