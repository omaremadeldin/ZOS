;==========================================
;			ZapperOS Memory 16-BIT
;------------------------------------------
;Done By Omar Emad Eldin
;Credits To : Mike
;==========================================
BITS 16

;==========================================
;GetMemorySize()
;------------------------------------------
;EAX<=Amount Of Installed Memory
;------------------------------------------
GetMemorySize:
	push ecx
	push edx
	
	xor	ecx, ecx								;ECX=0
	xor	edx, edx								;EDX=0
	
	mov	ax, 0xE801								;Function Number
	int	0x15									;Call BIOS
	jcxz .UseAX									;If CX=DX=0
	
	mov	ax, cx									;If Not Then
	mov	bx, dx									;Copy Them To AX And BX

.UseAX:
	pop	edx
	pop	ecx
	
	and eax,0xFFFF								;EAX=AX only
	and ebx,0xFFFF								;EBX=BX only
	
	add eax,1024								;Add 1 MB
	imul ebx,64									;Multiply EBX By 64
	add eax,ebx									;EAX=Final Result
ret

;==========================================
;GetMemoryMap()
;------------------------------------------
;ES:DI<=Buffer To Fill
;BP=>No. Of Entries
;CF On Error
;------------------------------------------
GetMemoryMap:
	push eax
	push ebx
	push ecx
	push edx
	
	xor ebx,ebx
	xor bp,bp
	
	mov eax,0xE820
	mov edx,'PAMS'
	mov ecx,24
	
	int 0x15
	jc .Error
	
	cmp eax,'PAMS'
	jne .Error
	
	cmp ebx,0x00
	je .Error
	
	jmp .Start
	
	.NextEntry:
		mov edx,'PAMS'
		mov ecx,24
		mov eax,0xE820
		
		int 0x15
		
	.Start:
		jcxz .SkipEntry
	
		mov ecx,DWORD[es:di+8]
		cmp ecx,0x00
		jne .GoodEntry
		
		mov ecx,DWORD[es:di+12]
		jecxz .SkipEntry
		
	.GoodEntry:
		inc bp
		add di,24
		
	.SkipEntry:
		cmp ebx,0x00
		jne .NextEntry
	pop eax
	pop ebx
	pop ecx
	pop edx
	
	clc
	
ret

.Error:
	pop eax
	pop ebx
	pop ecx
	pop edx
	stc
ret
;==========================================