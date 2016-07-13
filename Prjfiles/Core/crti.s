GLOBAL _init
GLOBAL _fini

SECTION .init

_init:
	push ebp
	mov ebp, esp

SECTION .fini

_fini:
	push ebp
	mov ebp, esp