[bits 32]

section .text
align 4
global run_kernel_task

run_kernel_task:
    cli                             ; disable external interrupts
    mov     eax, [esp+4]            ; load address of registers_t into eax

    ; restore all the registers except eax
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


