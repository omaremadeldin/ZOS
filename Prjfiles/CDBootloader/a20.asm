;==========================================
;				ZapperOS A20
;------------------------------------------
;Done By Omar Emad Eldin
;Credits To : Mike
;==========================================
BITS 16

;==========================================
;EnableA20()
;------------------------------------------
EnableA20:
	cli
pushad	;											;Disable Keyboard
	call WaitInput
	mov al,0xAD
	out 0x64,al
	call WaitInput
	;											;Tell Controller To Read Output Port
	mov al,0xD0
	out 0x64,al
	call WaitOutput
	;											;Get Output Port Data
	in al,0x60
	push eax
	call WaitInput
	;											;Tell Controller To Write Output Port
	mov al,0xD1
	out 0x64,al
	call WaitInput
	;											;Set Output Port Data
	pop eax
	or al,2										;Set Bit-1 (Enable A20 Gate)
	out 0x60,al
	;											;Enable Keyboard
	call WaitInput
	mov al,0xAE
	out 0x64,al
	
	Call WaitInput
	
	popad
	sti
ret

WaitInput:										;Wait For Input Buffer To Be Clear
	in al,0x64
	test al,2
	jnz WaitInput
ret

WaitOutput:										;Wait For Output Buffer To Be Clear
	in al,0x64
	test al,1
	jz WaitOutput
ret
;==========================================