;==========================================
;			ZapperOS ISO9660
;------------------------------------------
;Done By Omar Emad Eldin
;Credits To : Mike
;==========================================
BITS 16

%define BYTES_PER_SECTOR	2048

;==========================================
;ReadSectors()
;------------------------------------------
;ES:SI=>Buffer To Read In
;EBX=>Starting Sector
;ECX=>Size in bytes
;BP=0 Success , BP = -1 Failure
;------------------------------------------
ReadSectors:
	pushad
	push es
	
	push si										;Save SI
	
	mov ah,0x48									;Function No.
	mov dl,BYTE[BootDrive]						;Boot Drive
	mov si,BootDriveParams						;Buffer To Put Params In
	
	int 0x13									;Invoke BIOS
	
	pop si										;Restore SI
	
	jc .Error									;If Carry Is Set Then Error
	
	movzx eax,WORD[BootDriveParams+0x18]		;EAX = Bytes Per Sector
	mov WORD[BytesPerSector],ax					;Save It For Later

	mov DWORD[cdStart],ebx						;Starting Block
	
	xor edx,edx									;Prepare EDX For Division
	xchg eax,ecx								;AX = Data Size, CX = Bytes Per Sector
	div ecx										;AX = AX / CX
	mov ecx,eax									;CX = No. OF Blocks
	
	cmp edx,0x00								;If EDX=0
	je .NextStep								;Go To Next Step
	
	add ecx,1									;If EDX!=0 Then Add 1 To ECX
	
	.NextStep:
	
	mov WORD[cdNoBlocks],cx						;No. Of Blocks To Read
	mov WORD[cdSegment],es						;Buffer (Segment)
	mov WORD[cdOffset],si						;Buffer (Offset)
	
	mov ah,0x42									;Function No. (Int 0x13 Extension)
	mov dl,BYTE[BootDrive]						;Boot Drive
	mov si,DAP_CD								;Disk Address Packet
	
	int 0x13									;Invoke BIOS

	jc .Error									;If Status is not zero Then Error
	
	pop es
	popad
	mov bp,0x00									;If Successfull Then BP=0
ret

	.Error:
		pop es
		popad
		mov bp,-1								;If Failed Then BP=-1
ret
	
;==========================================
;LoadPVD()
;------------------------------------------
;ES:SI<=Buffer To Read In
;CF on Error
;------------------------------------------
LoadPVD:
pushad	
	mov ebx,0x10
	mov ecx,0x0800
	call ReadSectors
	
	cmp bp,-1
	je .Error
	
	popad
	
	clc
ret

.Error:
	popad
	stc
ret
;==========================================
;LoadRDT()
;------------------------------------------
;ES:DI<=Buffer To Read In
;ES:SI=>Address Of PVD
;CF on Error
;------------------------------------------
LoadRDT:
pushad	
	add si,0x9C
	mov ebx,DWORD[si+2]
	mov ecx,DWORD[si+10]
	mov si,di
	call ReadSectors
	
	cmp bp,-1
	je .Error
	
	popad
	
	clc
ret

.Error:
	popad
	stc
ret
;==========================================
;FindFile()
;------------------------------------------
;SI<=Address Of File Directory Record
;BX=>Address Of FileName
;CX=>Length Of FileName
;CF On Error
;------------------------------------------
FindFile:
	push eax
	push ebx
	push ecx
	
	mov si,0x7E00
	call LoadPVD
	jc .Error	
	
	mov di,0x7E00
	call LoadRDT
	jc .Error
	
	.Loop:
		mov ah,BYTE[di]							;Load 1st Byte = Size Of Record
		cmp ah,0x00								;If Size Of Record Is Zero Then
		je .Error								;Error
	
		cmp ah,0x22								;If Record Is Root Directory Record
		je .ContinueLoop						;Then Skip It & Continue Looping
		
		mov al,BYTE[di+32]						;Add (32) To Get File\Folder Name Length
		cmp al,cl								;If Not Same Length As KRNLDR Image
		jne .ContinueLoop						;Then Skip It & Continue Looping
		
		mov si,bx								;SI=ImageName
		cld										;Clear Direction Bit
		push di									;Save DI
		add di,33								;Add (33) To Get File\Folder Name
		rep cmpsb								;Compare SI And DI
		pop di									;Restore DI
		je .LoopEnd								;If Successful Then Continue Next Step

	.ContinueLoop:								;When Continuing Loop
		movzx dx,ah								;Add AH To DI (BaseAddress)
		add di,dx								;To Get Next Record
		jmp .Loop								;Then Loop
	.LoopEnd:
	
	mov si,di
	
	pop ecx
	pop ebx
	pop eax
	
	clc
ret

.Error:
	pop ecx
	pop ebx
	pop eax
	stc
ret
;==========================================
;LoadFile()
;------------------------------------------
;ES:SI<=Address Of File Directory Record
;EDI<=Buffer To Load File In
;EAX<=File Size In Bytes
;CF on Error
;------------------------------------------
LoadFile:
	pushad
	
	mov eax,DWORD[es:si+10]
	mov DWORD[BytesToRead],eax
	mov ebx,DWORD[es:si+2]
	
	push eax
	xor edx,edx
	mov ecx,0x10000
	div ecx
	mov ecx,eax
	pop eax
	
	cmp ecx,0x00
	je .EndLoop
	
	push edx
	mov edx,0x00
	
	.Loop:
		push ecx
		push si
		mov ecx,0x10000
		mov si,FileBuffer
		call ReadSectors
		pop si
		pushad
		xor ecx,ecx
		.LoopCopy:
		mov esi,FileBuffer
		mov al,BYTE[esi + ecx]
		mov BYTE[edi + ecx],al
		inc ecx
		cmp ecx,0x10000
		jne .LoopCopy
		popad
		pop ecx
	add edi,0x10000
	add ebx,0x20
	inc edx
	cmp edx,ecx
	jne .Loop
	
	pop edx
	
	.EndLoop:
	
	push ecx
	push si
	mov ecx,edx
	mov si,FileBuffer
	call ReadSectors
	pop si
	pushad
	xor ecx,ecx
	.LoopOnceCopy:
	mov esi,FileBuffer
	mov al,BYTE[esi + ecx]
	mov BYTE[edi + ecx],al
	inc ecx
	cmp ecx,0x10000
	jne .LoopOnceCopy
	popad
	pop ecx
	
	;cmp bp,-1
	;je .Error
	
	popad
	mov eax,DWORD[BytesToRead]
	
	clc
ret

.Error:
	popad
	stc
ret
;==========================================

DAP_CD:
cdDAPsize	DB 0x10
cdReserved	DB 0x00
cdNoBlocks	dw 0x0000
cdOffset	dw 0x0000
cdSegment	dw 0x0000
cdStart		dd 0x0000
cdExtra		dd 0x0000

BootDriveParams:
BufferSize 	dw 0x1E
TIMES 29 	DB 0

BytesPerSector	dw 0x0000
BytesToRead		dd 0x00