#==========================================
#
#		  ZapperOS - Core MAKEFILE
#
#==========================================
#By Omar Emad Eldin
#==========================================

CC = i386-elf-g++
CFLAGS = -c -std=c++0x -O0 -ffreestanding -nostdlib -Wall -Wextra -Werror -Wno-packed-bitfield-compat -fcheck-new -fno-omit-frame-pointer -masm=intel -fno-exceptions -fno-unwind-tables -fno-rtti -fno-common -ffriend-injection
INCLUDES = -Iinclude

LD = i386-elf-ld
LDFLAGS = -T link.ld

OBJCOPY = i386-elf-objcopy

AS = nasm
ASFLAGS = -f elf

FILESYSSRCS = hal/filesystems/fat.cpp
VMODESSRCS = hal/vmodes/vmode7.cpp
HALUTILSSRCS = hal/utils/format.cpp
HALSRCS = $(HALUTILSSRCS) $(FILESYSSRCS) hal/vmgr.cpp hal/pmgr.cpp hal/ide.cpp hal/pci.cpp $(VMODESSRCS) hal/video.cpp hal/crt.cpp hal/heap.cpp hal/vmm.cpp hal/pmm.cpp hal/kybrd.cpp hal/pit.cpp hal/pic.cpp hal/exceptions.cpp hal/idt.cpp hal/gdt.cpp hal/hal.cpp

CPPSRCS = $(HALSRCS) kmain.cpp
CPPOBJS = $(CPPSRCS:.cpp=.o)

ASMSRCS = hal/aih.s hal/asms.s crtn.s crti.s crt0.s
ASMOBJS = $(ASMSRCS:.s=.o)

OUT = KRNL1.SYS

all:	$(ASMOBJS) $(CPPOBJS)
	$(LD) $(LDFLAGS) crt0.o crti.o crtbegin.o hal/aih.o hal/asms.o $(CPPOBJS) ../ZLibC/zlibc.a crtend.o crtn.o -o $(OUT)
	$(OBJCOPY) -S -O binary $(OUT) $(OUT)
	
crt0.o:		crt0.s	
crti.o:		crti.s	
crtn.o:		crtn.s
hal/asms.o:	hal/asms.s
hal/aih.o:	hal/aih.s
$(ASMOBJS):
	$(AS) $(ASFLAGS) $? -o $@
	
kmain.o:				kmain.cpp

hal/hal.o:				hal/hal.cpp
hal/gdt.o:				hal/gdt.cpp
hal/idt.o:				hal/idt.cpp
hal/exceptions.o:		hal/exceptions.cpp
hal/pic.o:				hal/pic.cpp
hal/pit.o:				hal/pit.cpp
hal/kybrd.o:			hal/kybrd.cpp
hal/pmm.o:				hal/pmm.cpp
hal/vmm.o:				hal/vmm.cpp
hal/heap.o:				hal/heap.cpp
hal/crt.o:				hal/crt.cpp
hal/video.o:			hal/video.cpp
hal/pci.o:				hal/pci.cpp
hal/ide.o:				hal/ide.cpp
hal/pmgr.o:				hal/pmgr.cpp
hal/vmgr.o:				hal/vmgr.cpp

hal/vmodes/vmode7.o:	hal/vmodes/vmode7.cpp

hal/filesystems/fat.o: 	hal/filesystems/fat.cpp

hal/utils/format.o:		hal/utils/format.cpp

$(CPPOBJS):
	$(CC) $(CFLAGS) $(INCLUDES) $? -o $@
	
clean:	
	rm -r -f $(CPPOBJS)
	rm -r -f $(ASMOBJS)
	rm -r -f $(OUT)