[bits 32]

extern main

section .text
	; push argv
	; push argc
;'	push 0
;	push 0
	call main
	; main has returned, eax is return value
	jmp  $    ; loop forever
