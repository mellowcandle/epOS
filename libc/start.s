[bits 32]

extern main
extern stdlib_init

section .text
	; push argv
	; push argc
;'	push 0
;	push 0
	call stdlib_init
	call main
	; main has returned, eax is return value
	jmp  $    ; loop forever
