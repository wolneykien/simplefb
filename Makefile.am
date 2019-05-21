
bin_PROGRAMS = simplefb
simplefb_SOURCES = simplefb.c
simplefb_CFLAGS = -Wall -Werror
simplefb_LDADD = -lX11

# 320 x 240 = 76800
#fb0:
#	dd if=/dev/urandom of=fb0 count=76800 bs=2

#test: test.o vga.o
#	gcc -o test test.o vga.o
#
#test.o: test.c
#	gcc -c test.c -o test.o
#
#vga.o: vga.c
#	gcc -c vga.c -o vga.o
#
#clean:
#	rm -f vga.o test.o test fb0 simplefb
