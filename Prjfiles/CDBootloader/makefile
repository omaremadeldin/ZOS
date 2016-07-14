#==========================================
#
#	   ZapperOS - Bootloader MAKEFILE
#
#==========================================
#By Omar Emad Eldin
#==========================================

AS = nasm

SRCS = BOOTLDR.asm KRNLDR.asm
OUT = BOOTLDR.SYS KRNLDR.SYS

all: $(OUT)

BOOTLDR.SYS:	BOOTLDR.asm
KRNLDR.SYS:		KRNLDR.asm
$(OUT):
	$(AS) $? -o $@
	
clean:
	rm -r -f $(OUT)