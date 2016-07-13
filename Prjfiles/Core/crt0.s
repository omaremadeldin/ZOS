;;==========================================
;;
;;		   ZapperOS - Entry Point
;;
;;==========================================

[BITS 32]

%define CODE_SELECTOR	0x08
%define DATA_SELECTOR	0x10

%define KERNEL_PBASE 	0x100000
%define KERNEL_VBASE 	0xC0000000
%define PAGE_SIZE		4096

%define PAGE_DIRECTORY_ACCESS	0xFFFFF000
%define PAGE_TABLE_ACCESS		0xFFFFE000

GLOBAL _start

EXTERN kmain

GLOBAL BOOT_INFO
GLOBAL KERNEL_SIZE

GLOBAL ctorsInit

EXTERN _init

EXTERN __cxa_finalize

GLOBAL STACK_START
GLOBAL STACK_END

SECTION .edata

pageDirectory 	TIMES 1024 DD 0
pageTable1		TIMES 1024 DD 0		;First 4MBs Identity Mapping
pageTable2		TIMES 1024 DD 0		;4MBs from 0x100000 to 0xC0000000
pageTable3		TIMES 1024 DD 0		;Only one block for the page directory access

%define PAGE_DIRECTORY 	(pageDirectory - KERNEL_VBASE + KERNEL_PBASE)
%define PAGE_TABLE1		(pageTable1 - KERNEL_VBASE + KERNEL_PBASE)
%define PAGE_TABLE2		(pageTable2 - KERNEL_VBASE + KERNEL_PBASE)
%define PAGE_TABLE3		(pageTable3 - KERNEL_VBASE + KERNEL_PBASE)

BOOT_INFO		DD 0
KERNEL_SIZE		DD 0

SECTION .etext

_start:
	mov ax, DATA_SELECTOR
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov esp, (STACK_END - KERNEL_VBASE + KERNEL_PBASE)

	call SetupPaging
	
	;Used to convert to the virtual base of the kernel
	push CODE_SELECTOR
	push .Paging
	retf
	.Paging:
	
	mov esp, STACK_END
	mov DWORD[KERNEL_SIZE], ecx
	mov DWORD[BOOT_INFO], ebx
	
	call kmain
	
	sub esp, 0x04
	mov DWORD[esp], 0x00
	call __cxa_finalize
	add esp, 0x04
	
	cli
	hlt

;==========================================
;initCtors()
;------------------------------------------
ctorsInit:
	pushad
	call _init
	popad
ret	

;==========================================
;Memclr()
;------------------------------------------
;Clears a portion of memory
;------------------------------------------
;EDI=>Address
;ECX=>Size
;------------------------------------------
Memclr:
	pushad
	mov eax,0x00
	rep stosb
	popad
ret

;==========================================
;GetDirectoryIndex()
;------------------------------------------
;EBX=>Virtual Address
;EBX<=Directory Index
;------------------------------------------
GetDirectoryIndex:
	shr ebx,0x16
	and ebx,0x3FF
ret
;==========================================
;GetTableIndex()
;------------------------------------------
;EBX=>Virtual Address
;EBX<=Table Index
;------------------------------------------
GetTableIndex:
	shr ebx,0x0C
	and ebx,0x3FF
ret

;==========================================
;MapPage()
;------------------------------------------
;EAX=>Physical Address of Page Directory
;EDX=>Address Of New PageTable
;ESI=>Physical Address
;EDI=>Virtual Address
;CF=>On Error
;------------------------------------------
MapPage:
	pushad

	mov ebx,edi									;EBX=Virtual Address To Map
	call GetDirectoryIndex						;EBX=DirectoryPage Index
	
	;mov ecx,DWORD[eax + (ebx*4)]				;ECX=EAX(Address Of Default DirectoryTable) + (DirectoryTable Index * Size Of One Entry)
	
	or edx,0x07									;(EDX Must be 4-Byte Aligned)Set Present & Read/Write Flags & US Flag
	;or edx,0x03								;(EDX Must be 4-Byte Aligned)Set Present & Read/Write Flags
	mov DWORD[eax + (ebx*4)],edx				;Store PageDirectory Entry
	mov ecx,edx									;ECX=PageDirectory Entry

	and ecx,0xFFFFF000							;ECX=Physical Address Of PageTable
	mov ebx,edi									;EDI=Virtual Address
	call GetTableIndex
	
	mov DWORD[ecx + (ebx*4)],esi				;Store PageTable Entry
	;or DWORD[ecx + (ebx*4)],3
	or DWORD[ecx + (ebx*4)],7					;Set Present & Read/Write Flags & US Flag
	
	popad
	clc
ret

.Error:
	popad
	stc
ret


;==========================================
;SetupPaging()
;------------------------------------------
SetupPaging:
	pushad
	mov edi, PAGE_DIRECTORY
	mov ecx, PAGE_SIZE
	call Memclr

	mov edi, PAGE_TABLE1
	mov ecx, PAGE_SIZE
	call Memclr

	mov edi, PAGE_TABLE2
	mov ecx, PAGE_SIZE
	call Memclr

	mov edi, PAGE_TABLE3
	mov ecx, PAGE_SIZE
	call Memclr

	mov ecx,1024
	mov esi, 0x00000000
	mov edi, 0x00000000
	.Loop1:
		mov eax, PAGE_DIRECTORY
		mov edx, PAGE_TABLE1
		call MapPage
		add esi, PAGE_SIZE
		add edi, PAGE_SIZE
	loop .Loop1

	mov ecx,1024
	mov esi, 0x00100000
	mov edi, 0xC0000000
	.Loop2:
		mov eax, PAGE_DIRECTORY
		mov edx, PAGE_TABLE2
		call MapPage
		add esi, PAGE_SIZE
		add edi, PAGE_SIZE
	loop .Loop2

	mov eax, PAGE_DIRECTORY
	mov edx, PAGE_TABLE3
	mov esi, PAGE_DIRECTORY
	mov edi, PAGE_DIRECTORY_ACCESS
	call MapPage

	mov eax, PAGE_DIRECTORY
	or 	eax, 0x03
	mov cr3, eax

	mov eax, cr0
	or 	eax, 0x80000000
	mov cr0, eax
	
	popad
ret

SECTION .bss
	STACK_START:
		RESB	(8*1024)
	STACK_END: