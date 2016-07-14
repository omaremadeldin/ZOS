;==========================================
;			ZapperOS STDIO 32-BIT
;------------------------------------------
;Done By Omar Emad Eldin
;==========================================
BITS 32

%define VideoMemory 0xB8000
%define	Columns	80
%define Rows 25
%define	CharAttrib 00011111b

CurX DB 0
CurY DB 0

;==========================================
;PrintCh32()
;------------------------------------------
;BL<=Character To Print
;------------------------------------------
PrintCh32:
pushad
	mov edi,VideoMemory
	
	xor eax,eax
	
	;											;Current Position = X + Y(Columns)
	;											;Mode 7 Has 2 Bytes Per Character
	
	mov ecx,Columns * 2
	mov al,BYTE[CurY]							;Get Y Position
	mul ecx										;Y * Columns
	push eax									;Save Product Of Multiplication
	
	mov al,BYTE[CurX]
	mov cl,0x02
	mul cl
	pop ecx										;ECX = The EAX We Saved Earlier
	add eax,ecx									;Offset Address
	
	xor ecx,ecx
	add edi,eax									;Add Offset Address To Base Address
	
	cmp bl,0x0A									;Is New-Line Character
	je .Row										;Go To Next Row
	
	mov dl,bl									;Character
	mov dh,CharAttrib							;Character Attribute
	mov word [edi],dx							;Write To Video Display
	
	inc BYTE[CurX]								;Next Character
	jmp .Done
	
	.Row:
		mov BYTE[CurX],0x00						;Reset X Position
		inc BYTE[CurY]							;New Line
		
	.Done:
	popad
ret
;==========================================
;Print32()
;------------------------------------------
;EBX<=Address Of String To Print
;------------------------------------------
Print32:
pushad
	push ebx
	pop edi										;EDI = EBX
	.Loop:
		mov bl,BYTE[edi]						;Get Current Character
		cmp bl,0x00								;Is Null-Terminator Character
		je .Done
		
		call PrintCh32							;Print Current Character
		
		inc edi									;Next Character
		jmp .Loop
	.Done:
		mov bh,BYTE[CurY]						;Get Current Position
		mov bl,BYTE[CurX]
		call MoveCursor							;Update Hardware Cursor
		
popad
ret
;==========================================
;MoveCursor()
;------------------------------------------
;BH<=Y Position
;BL<=X Position
;------------------------------------------
;Current Position = X + Y(Columns)
;------------------------------------------
MoveCursor:
pushad
	
	xor eax,eax									;EAX = 0
	mov ecx,Columns
	mov al,bh									;Get Y Position
	mul ecx										;Multiply Y*Columns
	add al,bl									;Add X Position To The Result
	mov ebx,eax
	;											;Set Low Byte Index To VGA Register
	mov al,0x0F
	mov dx,0x03D4
	out dx,al
	
	mov al,bl
	mov dx,0x03D5
	out dx,al
	;											;Set High Byte Index To VGA Register
	xor eax,eax									;EAX = 0
	
	mov al,0x0E
	mov dx,0x03D4
	out dx,al
	
	mov al,bh
	mov dx,0x03D5
	out dx,al
	
popad
ret
;==========================================
;ClearScreen()
;------------------------------------------
ClearScreen:
pushad
	
	cld											;Forward Direction
	mov edi,VideoMemory
	mov cx,(Columns * Rows)						;Counter For REP (Columns * Rows)
	mov ah,CharAttrib
	mov al,' '
	rep stosw									;Store AX in EDI
	mov BYTE[CurY],0x00							;Reset Cursor Position
	mov BYTE[CurX],0x00
	
popad
ret
;==========================================
;GoToXY()
;------------------------------------------
;AH<=Y Position
;AL<=X Position
;------------------------------------------
GoToXY:
pushad
	;											;Just Set The Cursor Positions
	mov BYTE[CurY],ah
	mov BYTE[CurX],al
	
popad
ret
;==========================================