;==========================================
;			ZapperOS Bootloader
;------------------------------------------
;Done By Omar Emad Eldin
;Credits To : Mike
;==========================================

BITS 16											;16-Bit Real Mode
ORG 0x0000										;Base Address
Start:jmp Main									;Jump To The Start Of Bootloader

%strlen ImageSize 	'KRNLDR.SYS'
ImageName DB 'KRNLDR.SYS'
%define BaseAddress 0x200
%define LoadAddress 0x500

;==========================================
;Prints A Simple String
;------------------------------------------
;SI=>String To Print
;------------------------------------------
Print:
	lodsb
	cmp al,0x00
	je PrintDone				
	mov bx,0x0000
	mov ah,0x0E					
	int 0x10
	jmp Print
PrintDone:
	ret
;==========================================
;Reads A Group Of Sectors Into A Buffer
;------------------------------------------
;ES:SI=>Buffer To Read In
;EBX=>Starting Sector
;CX=>Number Of Sectors
;BP=0 Success , BP = -1 Failure
;------------------------------------------
ReadSectors:
pushad
	
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
	xchg ax,cx									;AX = Data Size, CX = Bytes Per Sector
	div cx										;AX = AX / CX
	mov cx,ax									;CX = No. OF Blocks
	
	cmp cx,0x00									;If No. Of Blocks < 1
	jne .NextStep								;If Not Then Go On
	
	mov cx,1									;If True Then CX=1
	
	.NextStep:
	
	mov WORD[cdNoBlocks],cx						;No. Of Blocks To Read
	mov WORD[cdSegment],es						;Buffer (Segment)
	mov WORD[cdOffset],si						;Buffer (Offset)
	
	mov ah,0x42									;Function No. (Int 0x13 Extension)
	mov dl,BYTE[BootDrive]						;Boot Drive
	mov si,DAP_CD								;Disk Address Packet
	
	int 0x13									;Invoke BIOS
	
	jc .Error									;If Carry Then Error
	
popad
	mov bp,0x00									;If Successfull Then BP=0
ret
	.Error:
	popad
		mov bp,-1								;If Failed Then BP=-1
ret
	
;==========================================
Main:
	cli
	;											;Setup Segment Registers To 0x0000:0x7C00
	mov ax,0x07C0								
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	;											;Relocating Stack
	xor ax,ax									
	mov ss,ax
	mov sp,0xFFFF	
	sti
	;											;Save Boot Drive
	mov BYTE[BootDrive],dl
	;											;Print Welcome Message
	mov si,WelcomeMsg
	call Print
	
	.LoadPVD:									;Load PVD (Primary Volume Descriptor)
		mov ebx,0x10							;Starting Block
		mov cx,0x0800							;No. Of Bytes To Read
		mov si,BaseAddress						;Buffer
		call ReadSectors						;Read Sectors
		
		cmp bp,-1
		je Failure
		
	.LoadRDT:									;Load RDT (Root Directory Table)
		mov si,(BaseAddress+0x9C)				;(BaseAddress + 0x9C) The Address Of Root Directory Entry
		mov ebx,DWORD[si+2]						;Starting Block
		mov ecx,DWORD[si+10]					;No. Of Blocks To Read
		mov si,BaseAddress						;Buffer
		call ReadSectors						;Read Sectors
		
		cmp bp,-1								;On Failure Code
		je Failure								;Print Failure Message
	
	.FindDirectoryRecord:
		mov di,BaseAddress						;DI=BaseAddress
		.Loop:
			mov ah,BYTE[di]						;Load 1st Byte = Size Of Record
			cmp ah,0x00							;If Size Of Record Is Zero Then
			je Failure							;Print Failure Message
		
			cmp ah,0x22							;If Record Is Root Directory Record
			je .ContinueLoop					;Then Skip It & Continue Looping
			
			mov al,BYTE[di+32]					;Add (32) To Get File\Folder Name Length
			cmp al,ImageSize					;If Not Same Length As KRNLDR Image
			jne .ContinueLoop					;Then Skip It & Continue Looping
			
			mov cx,ImageSize					;Use KRNLDR Image Name Length As Counter
			mov si,ImageName					;SI=ImageName
			cld									;Clear Direction Bit
			push di								;Save DI
			add di,33							;Add (33) To Get File\Folder Name
			rep cmpsb							;Compare SI And DI
			pop di								;Restore DI
			je .LoopEnd							;If Successful Then Continue Next Step

		.ContinueLoop:							;When Continuing Loop
			movzx dx,ah							;Add AH To DI (BaseAddress)
			add di,dx							;To Get Next Record
			jmp .Loop							;Then Loop
		.LoopEnd:
	
	.LoadImage:									;Start Loading Image
		mov ebx,DWORD[di+2]						;Starting Block
		mov ecx,DWORD[di+10]					;No. Of Blocks To Read
		mov ax,0x000							;AX = 0x0000
		mov es,ax								;ES = AX = 0x0000
		mov si,LoadAddress						;SI = 0x0500
		call ReadSectors						;Read KRNLDR To Memory At (0x0000:0x0500)
	
		cmp bp,-1								;On Failure Code
		je Failure								;Print Failure Message
	
	Done:										;If Everything Is OK
		mov dl,BYTE[BootDrive]
		push WORD 0x0000						;Then Jump To KRNLDR
		push WORD LoadAddress					;(0x0000:0x0500)
		retf
		
	Failure:
		mov si,FailureMsg
		call Print
		mov ah,0x00
		int 0x16
		int 0x19


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

BootDrive		DB 0x00
BytesPerSector	dw 0x0000

FailureMsg		DB	'**A Fatal Error Occured While Booting**',0x0D,0x0A,0x00
WelcomeMsg 		DB  'Welcome To ZapperOS!',0x0D,0x0A,0x00
	
TIMES 2046-($-$$) DB 0
DW 0xAA55
