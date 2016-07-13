;==========================================
;			ZapperOS PRINTF
;------------------------------------------
;Done By Omar Emad Eldin
;Credits To : Mike
;==========================================
BITS 16

;==========================================
;xtoa()
;------------------------------------------
;EDX=>Hex To Convert
;------------------------------------------
HexSBuffer		TIMES 16 DB 0

xtoa:
	pushad
	
	mov di,HexSBuffer
	push di
	mov ax,0x00
	mov cx,0x04
	rep stosd
	pop di
	
	cmp edx,0x00								;If EDX=0
	jne .Start									;If False Then Jump To Start
	
	mov BYTE[di],'0'							;If True Then Print '0' Character
	mov BYTE[di+1],0x00
	
	popad
ret
	
	.Start:
		
		mov di,HexSBuffer						;ESI=Address To Put Converted Value In
		
		.Loop:
			cmp edx,0x00						;If EDX=0
			je .Done							;If True Then End Loop

			xor eax,eax							;EAX=0
			xchg eax,edx						;EAX=Quotient , EDX=Remainder
			
			mov ecx,0x10						;Hexadecimals Are Base-16
			div ecx								;Divide By Base (16)
			xchg eax,edx						;EAX=Remainder , EDX=Quotient
			
			cmp al,0x09							;Compare Byte With 9
			jg .HexNumbers						;If Greater Than 9 Then Process Hex No.s
			
			.NormalNumbers:						;Format Normal No.s
				or al,00110000b
				jmp .EndFormat
			
			.HexNumbers:						;Format Hex No.s
				sub al,0x09
				or al,01000000b
				
			.EndFormat:
			
			mov BYTE[di],al						;Move Converted Character To Buffer
			
			inc di								;Increment Source Address
			jmp .Loop							;Loop
		.Done:
	
		mov si,HexSBuffer						;ESI=Address Of Buffer
		mov di,HexSBuffer						;EDI=Address Of Buffer
		call Strreverse							;Reverse String
	
	popad
ret

;==========================================
;Strreverse()
;------------------------------------------
;ESI=>Address Of Source String
;EDI=>Address Of Destination String
;------------------------------------------
Strreverse:
	pushad
	
	xor cx,cx
	
	mov ax,si	
	push 0x00
	.Loop1:
		cmp BYTE[si],0x00
		je .Loop1End
		
		movzx bx,BYTE[si]
		push bx
		inc si
		inc cx
		
		jmp .Loop1
	.Loop1End:	
	mov si,ax
	
	inc cx
	.Loop2:
		pop ax
		mov BYTE[di],al
		inc di
		loop .Loop2
	
	popad
ret
