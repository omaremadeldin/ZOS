;==========================================
;		ZapperOS Kernel Loader 16-Bit
;------------------------------------------
;Done By Omar Emad Eldin
;==========================================
BITS 16

ORG 0x500

jmp Main

%include "stdio16.asm"
%include "printf.asm"
%include "gdt.asm"
%include "a20.asm"
%include "common.asm"
%include "iso9660.asm"
%include "memory.asm"

;												;Data Section

LoadingMsg 	DB 'Loading Operating System....',0x0D,0x0A,0x00
FailureMsg1 DB '**FATAL ERROR**Cannot find KRNL1.SYS*',0x0D,0x0A,0x00
FailureMsg2 DB '**FATAL ERROR**Cannot load KRNL1.SYS*',0x0D,0x0A,0x00
BootDrive	DB 0x00

oldgdt:
dw 0
dd 0

gdtinfo:
   dw gdt_end - gdt - 1   ;last byte in table
   dd gdt                 ;start of table
 
gdt         dd 0,0        ; entry 0 is always unused
flatdesc    db 0xff, 0xff, 0, 0, 0, 10010010b, 11001111b, 0
gdt_end:

Main:
	cli
	;											;Setup Segments
	xor ax,ax
	mov ds,ax
	mov es,ax
	mov gs,ax
	mov fs,ax
	;											;Creating Stack
	mov ax,0x9000
	mov ss,ax
	mov sp,0xFFFF
	sti
	;
	mov BYTE[BootDrive],dl
	;											;Print Loading Screen
	mov si,LoadingMsg
	call Print16
	
	;											;Get Memory Size
	call GetMemorySize
	push eax									;Save It On Stack
	
	;											;Put Memory Map To (0x0000:0x1000)
	mov di,MMapAddress
	call GetMemoryMap
	
	and ebp,0xFFFF								;EBP=BP Only
	push ebp									;Save Entry Count On Stack
	
	;											;Install GDT
	call InstallGDT
	;											;Enable A20 Gate	
	call EnableA20
	;											;Change To The Selected Video Mode
	; mov ah,00h
	; mov al,13h
	; int 10h
	;											;Load Kernel Image
	cli
	push ds
	
	sgdt [oldgdt]			;preserve old gdt
	lgdt [gdtinfo]			;load temporary gdt
							;switch to protected mode
    mov  eax,cr0
	or   eax,0x01
	mov  cr0,eax

	jmp $+2                	;tell 386/486 to not crash

	mov bx,0x08          	;select descriptor 1
	mov ds,bx            	;8h = 1000b

	and al,0xFE            	;back to realmode
	mov cr0, eax          	;by toggling bit again
							;now we are in unreal mode
	pop ds
	sti
	
	mov bx,ImageName
	mov cx,ImageNameLength
	call FindFile
	jc Failure1
	
	mov edi,PModeBase
	call LoadFile
	jc Failure2
	
	mov DWORD[ImageSize],eax
	
	lgdt [oldgdt]								;restore the old gdt (return to real mode)
	
	jmp EnterPMode								;Enter Protected Mode
	
	Failure1:
	mov si,FailureMsg1
	call Print16								;Print Failure Message
	jmp Reset
	
	Failure2:
	mov si,FailureMsg2
	call Print16								;Print Failure Message
	
	Reset:
	mov ah,0x00									
	int 0x16									;Wait For Keypress
	int 0x19									;Reboot
	;											;Enter PMode
	EnterPMode:
		cli
		mov eax,cr0								;Set Bit-0 In CR0 To Enable PMode
		or eax,1
		mov cr0,eax
	
		jmp CodeDesc:PMODE						;Far Jump To Fix CS
	
;==========================================
;		ZapperOS Kernel Loader 32-Bit
;------------------------------------------
;Done By Omar Emad Eldin
;==========================================
BITS 32

%include "stdio32.asm"
%include "multiboot.asm"

boot_info:
istruc multiboot_info
	at multiboot_info.flags,				DD 0
	at multiboot_info.memoryLo,				DD 0
	at multiboot_info.memoryHi,				DD 0
	at multiboot_info.bootDevice,			DD 0
	at multiboot_info.cmdLine,				DD 0
	at multiboot_info.mods_count,			DD 0
	at multiboot_info.mods_addr,			DD 0
	at multiboot_info.syms0,				DD 0
	at multiboot_info.syms1,				DD 0
	at multiboot_info.syms2,				DD 0
	at multiboot_info.mmap_length,			DD 0
	at multiboot_info.mmap_addr,			DD 0
	at multiboot_info.drives_length,		DD 0
	at multiboot_info.drives_addr,			DD 0
	at multiboot_info.config_table,			DD 0
	at multiboot_info.bootloader_name,		DD 0
	at multiboot_info.apm_table,			DD 0
	at multiboot_info.vbe_control_info,		DD 0
	at multiboot_info.vbe_mode_info,		DW 0
	at multiboot_info.vbe_interface_seg,	DW 0
	at multiboot_info.vbe_interface_off,	DW 0
	at multiboot_info.vbe_interface_len,	DW 0
iend

bootloaderName DB "ZapperOS Bootloader",0x00

PMODE:
	pop	edx												;Restore memory map entry count
	pop ebp												;Restore memory size
	
	;													;Set Data Segments To 0x10 (Data Descriptor)
	mov ax,DataDesc
	mov ds,ax
	mov ss,ax
	mov es,ax
	mov esp,0x90000										;Stack Begins From 0x90000
	
	;													;Push same things on new stack with reverse order
	push edx
	push ebp

	or DWORD[boot_info + multiboot_info.flags],0x01		;Memory Size Flag
	or DWORD[boot_info + multiboot_info.flags],0x02		;Boot Drive Flag
	or DWORD[boot_info + multiboot_info.flags],0x40		;Memory Map Flag
	or DWORD[boot_info + multiboot_info.flags],0x200	;Bootloader Name Flag
	
	mov bl,BYTE[BootDrive]								;BL contains now boot device no.
	mov BYTE[boot_info + multiboot_info.bootDevice],bl
	
	pop eax												;Restore memory size
	
	mov ebx,eax
	and ebx,0xFFFF0000
	shr ebx,0x10										;Now AX contains memoryLo and BX contains memoryHi
	
	and eax,0xFFFF
	
	mov	DWORD[boot_info + multiboot_info.memoryHi],ebx
	mov	DWORD[boot_info + multiboot_info.memoryLo],eax
	
	pop eax												;Restore memory map entry count
	mov	DWORD[boot_info + multiboot_info.mmap_length],eax
	mov	DWORD[boot_info + multiboot_info.mmap_addr],MMapAddress
	;;TODO: support the real multiboot memory map instead of the BIOS one
	
	mov DWORD[boot_info + multiboot_info.bootloader_name],bootloaderName
	
	mov eax,0x2BADB002									;(EAX)Magic number to indicate that our kernel comply to the multiboot specification
	mov ebx,boot_info									;(EBX)Address of multiboot info structure
	mov ecx,DWORD[ImageSize]							;(ECX)Pass kernel size to our kernel
	
	jmp CodeDesc:PModeBase								;Load Kernel By Virtual Address