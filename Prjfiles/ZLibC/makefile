#==========================================
#
#		  ZapperOS - ZLIBC MAKEFILE
#
#==========================================
#By Omar Emad Eldin
#==========================================

CC = i386-elf-gcc
CFLAGS = -c -std=c99 -O0 -ffreestanding -fpic -nostdlib -Wall -Wextra -Wno-packed-bitfield-compat -fno-omit-frame-pointer -masm=intel -fno-exceptions -fno-unwind-tables -fno-common

AR = i386-elf-ar

SRCS = string.c ctype.c
OBJS = $(SRCS:.c=.o)
OUT = zlibc.a

all:	$(OBJS)
	$(AR) rc $(OUT) $(OBJS)

ctype.o:	ctype.c
string.o:	string.c
$(OBJS):
	$(CC) $(CFLAGS) $(INCLUDES) $? -o $@
	
clean:
	rm -r -f $(OBJS)
	rm -r -f $(OUT)