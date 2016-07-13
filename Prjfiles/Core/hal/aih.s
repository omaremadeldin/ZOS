;;==========================================
;;
;;		        ZapperOS - AIH
;;		  (Assembly Interrupt Handlers)
;;==========================================
;;Used Acronyms:
;;--------------
;;* AIH = Assembly Interrupt Handler
;;* PIT = Programmable Interval Timer
;;==========================================
[BITS 32]

GLOBAL aihDefault
GLOBAL aihPIT
GLOBAL aihKYBRD
GLOBAL aihIDE

EXTERN ihDefault
EXTERN ihPIT
EXTERN ihKYBRD
EXTERN ihIDE

; GLOBAL aihErrDivideByZero
; GLOBAL aihErrSingleStepDebug
; GLOBAL aihErrNMIPin
; GLOBAL aihErrBreakpointDebug
; GLOBAL aihErrOverflow
; GLOBAL aihErrBoundsRange
; GLOBAL aihErrUndefinedOPCode
; GLOBAL aihErrDoubleFault
; GLOBAL aihErrInvalidTSS
; GLOBAL aihErrSegmentNotPresent
; GLOBAL aihErrStackSegOverun
; GLOBAL aihErrGeneralProtectionFault
; GLOBAL aihErrPageFault

; EXTERN ihErrDivideByZero
; EXTERN ihErrSingleStepDebug
; EXTERN ihErrNMIPin
; EXTERN ihErrBreakpointDebug
; EXTERN ihErrOverflow
; EXTERN ihErrBoundsRange
; EXTERN ihErrUndefinedOPCode
; EXTERN ihErrDoubleFault
; EXTERN ihErrInvalidTSS
; EXTERN ihErrSegmentNotPresent
; EXTERN ihErrStackSegOverun
; EXTERN ihErrGeneralProtectionFault
; EXTERN ihErrPageFault

SECTION .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;Interrupt Handlers;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;Default Interrupt Handler
aihDefault:
	pushad
	call ihDefault
	popad
	iretd
	
;;PIT Interrupt Handler
aihPIT:
	pushad
	call ihPIT
	popad
	iretd

;;KYBRD Interrupt Handler
aihKYBRD:
	pushad
	call ihKYBRD
	popad
	iretd
	
;;IDE Interrupt Handler
aihIDE:
	pushad
	call ihIDE
	popad
	iretd

;;API Interrupt Handler
;aihAPI:
;	pushad
;	call ihAPI
;	popad
;	iretd
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;Exception Handlers;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; aihErrDivideByZero:
	; call ihErrDivideByZero
	; hlt
	
; aihErrSingleStepDebug:
	; call ihErrSingleStepDebug
	; hlt

; aihErrNMIPin:
	; call ihErrNMIPin
	; hlt

; aihErrBreakpointDebug:
	; call ihErrBreakpointDebug
	; hlt

; aihErrOverflow:
	; call ihErrOverflow
	; hlt

; aihErrBoundsRange:
	; call ihErrBoundsRange
	; hlt

; aihErrUndefinedOPCode:
	; call ihErrUndefinedOPCode
	; hlt

; aihErrDoubleFault:
	; call ihErrDoubleFault
	; hlt

; aihErrInvalidTSS:
	; call ihErrInvalidTSS
	; hlt

; aihErrSegmentNotPresent:
	; call ihErrSegmentNotPresent
	; hlt

; aihErrStackSegOverun:
	; call ihErrStackSegOverun
	; hlt

; aihErrGeneralProtectionFault:
	; call ihErrGeneralProtectionFault
	; hlt

; aihErrPageFault:
	; pop ebx		;Page Fault Error Code
	; pop ebx		;EIP
	
	; push ebx

	; mov eax,cr2
	; push eax
	
	; call ihErrPageFault
	
	; add esp,0x08
	
	; hlt