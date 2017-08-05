;
;	This is free and unencumbered software released into the public domain.
;
;	Anyone is free to copy, modify, publish, use, compile, sell, or
;	distribute this software, either in source code form or as a compiled
;	binary, for any purpose, commercial or non-commercial, and by any
;	means.
;
;	In jurisdictions that recognize copyright laws, the author or authors
;	of this software dedicate any and all copyright interest in the
;	software to the public domain. We make this dedication for the benefit
;	of the public at large and to the detriment of our heirs and
;	successors. We intend this dedication to be an overt act of
;	relinquishment in perpetuity of all present and future rights to this
;	software under copyright law.
;
;	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
;	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
;	IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
;	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
;	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
;	OTHER DEALINGS IN THE SOFTWARE.
;
;	For more information, please refer to <http://unlicense.org>
;

section .text
align 4

global run_kernel_task
global run_user_task

run_kernel_task:
	cli
	mov		eax, [esp+4] ; load task_register_t pointer to eax

    mov     ebx, [eax+4]
    mov     ecx, [eax+8]
    mov     edx, [eax+12]
    mov     ebp, [eax+16]
    mov     esi, [eax+20]
    mov     edi, [eax+24]

    ; restore the stack pointer
    mov     esp, [eax+32]

    ; push information for iret onto the stack
    push    DWORD [eax+36]          ; push EFLAGS
    push    DWORD [eax+40]          ; push the segment selector
    push    DWORD [eax+44]          ; push EIP

    mov     eax, [eax]              ; restore eax

    iret                            ; iret to return to the process

run_user_task:
    cli                             ; disable external interrupts
    mov     eax, [esp+4]            ; load address of registers_t into eax

    ; restore all the registers except eax
    mov     ebx, [eax+4]
    mov     ecx, [eax+8]
    mov     edx, [eax+12]
    mov     ebp, [eax+16]
    mov     esi, [eax+20]
    mov     edi, [eax+24]

    ; push information for iret onto the stack
    push    DWORD [eax+28]          ; push the SS onto the stack
    push    DWORD [eax+32]          ; push the ESP of the user stack
    push    DWORD [eax+36]          ; push EFLAGS
    push    DWORD [eax+40]          ; push the segment selector
    push    DWORD [eax+44]          ; push EIP

    ; move index for the data segment into data registers
    push    ecx
    mov     cx, [eax+28]
    mov     ds, cx
    mov     gs, cx
    mov     es, cx
    mov     fs, cx
    pop     ecx

    mov     eax, [eax]              ; restore eax

    iret                            ; iret into the given mode


