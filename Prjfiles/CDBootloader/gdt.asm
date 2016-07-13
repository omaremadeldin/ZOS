;==========================================
;				ZapperOS GDT
;------------------------------------------
;Done By Omar Emad Eldin
;Credits To : Mike
;==========================================
BITS 16

%define NullDesc 0x00
%define CodeDesc 0x08
%define DataDesc 0x10

;==========================================
;InstallGDT()
;------------------------------------------
InstallGDT:
pushad
	cli
	lgdt [GDT]
	sti
popad
ret
;==========================================

;												;Global Descriptor Table (GDT)
GDT_DATA:
	dd 0x0000
	dd 0x0000
;												;Code Descriptor
	dw 0xFFFF									;Limit (Low)
	dw 0x0000									;Base (Low)
	DB 0x00										;Base (Middle)
	DB 10011010b								;Access
	DB 11001111b								;Granularity
	DB 0x00										;Base (High)
;												;Data Descriptor
	dw 0xFFFF									;Limit (Low)
	dw 0x0000									;Base (Low)
	DB 0x00										;Base (Middle)
	DB 10010010b								;Access
	DB 11001111b								;Granularity
	DB 0x00										;Base (High)
GDT_END:
GDT:
	dw GDT_END - GDT_DATA - 1					;GDT Size - 1
	dd GDT_DATA									;Base Of GDT