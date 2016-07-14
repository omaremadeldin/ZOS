;==========================================
;			ZapperOS STDIO 16-BIT
;------------------------------------------
;Done By Omar Emad Eldin
;Credits To : Mike
;==========================================
BITS 16

;==========================================
;Print16()
;------------------------------------------
;SI<=Address Of String To Print
;------------------------------------------
Print16:
	pushad
	
	.Loop:
		lodsb
		cmp al,0x00
		je .Print16Done	
		mov bx,0x0000
	
		mov ah,0x0E					
		int 0x10
		jmp .Loop
		
	.Print16Done:
		popad
ret
