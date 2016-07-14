;;==========================================
;;
;;		        ZapperOS - ASMS
;;		  	   (Assembly  Stubs)
;;==========================================
[BITS 32]

GLOBAL gdtFlush
GLOBAL tssFlush
GLOBAL enterUsermode

gdtFlush:
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax

    jmp 0x8:.flushCS
.flushCS:

    ret

tssFlush:
	mov ax, 0x28
	ltr ax
	ret

enterUsermode:
	cli

	mov ax, 0x23
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push 0x23
	push esp
	pushfd
	push 0x1B

	lea eax, [.a]
	push eax

	iretd

	.a:
	int 0
	jmp $

	ret
