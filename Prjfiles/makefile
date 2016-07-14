#==========================================
#
#		  ZapperOS - ROOT MAKEFILE
#
#==========================================
#By Omar Emad Eldin
#==========================================

LIBC = ZLibC
BOOTLDR = CDBootloader
KRNL = Core

BOOTLDRFILES = BOOTLDR.SYS KRNLDR.SYS
KRNLFILES = KRNL1.SYS

OUT = ZapperOS.iso

BXRC = bochsrc.bxrc

all: libc bootloader kernel

all-run: all run

bochs:	$(OUT)
run:	$(OUT)
	bochs -q -f $(BXRC)
	
qemu:	$(OUT)
	qemu-system-i386 -boot d -cdrom ZapperOS.iso -m 256M -curses
	
qemu-gui:	$(OUT)
	qemu-system-i386 -boot d -cdrom ZapperOS.iso -m 256M

libc:
	cd $(LIBC) && \
	make
	
bootloader:
	cd $(BOOTLDR) && \
	make
	
kernel:	libc
	cd $(KRNL) && \
	make
	
$(OUT) iso:	bootloader kernel
	mkdir CDBOOT
	cp $(addprefix $(BOOTLDR)/, $(BOOTLDRFILES)) $(addprefix $(KRNL)/, $(KRNLFILES)) CDBOOT
	mkisofs -U -b BOOTLDR.SYS -hide BOOTLDR.SYS -V ZapperOS -iso-level 3 -no-emul-boot -boot-load-size 4 -o $(OUT) CDBOOT
	rm -r -f CDBOOT
	
clean:
	cd $(LIBC) && \
	make clean
	
	cd $(BOOTLDR) && \
	make clean
	
	cd $(KRNL) && \
	make clean
	
	rm -r -f CDBOOT
	rm -r -f $(OUT)