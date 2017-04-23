; by Andreas Galauner
;
; https://github.com/G33KatWork


[GLOBAL thread_getEflags]
thread_getEflags:
    pushf
    pop eax
    ret

[GLOBAL thread_switchToContext]
thread_switchToContext:
    mov ebx, dword[esp + 4]     ; Get thread info

;    mov eax, dword[ebx + 64]    ; Get new pagedir pointer
;    mov ecx, cr3                ; Get current pagedir pointer
;    cmp eax, ecx                ; Compare
;    je .noPagedirChange
;    mov cr3, eax                ; and set new if it differs from old

;.noPagedirChange:

    mov ecx, dword[ebx + 16]    ; restore registers
    mov edx, dword[ebx + 20]
    mov esp, dword[ebx + 28]
    mov ebp, dword[ebx + 32]
    mov esi, dword[ebx + 36]
    mov edi, dword[ebx + 40]

    mov eax, dword[ebx + 48]    ; restore segments
    mov es,  eax
    mov eax, dword[ebx + 44]
    mov ds,  eax
    mov eax, dword[ebx + 52]
    mov fs,  eax
    mov eax, dword[ebx + 56]
    mov gs,  eax
    mov eax, dword[ebx + 60]
    mov ss,  eax

    mov eax, dword[ebx + 12]    ; finally restore eax

    push dword[ebx + 8]         ; push eflags
    push dword[ebx + 4]         ; push cs
    push dword[ebx + 0]         ; push eip

    push dword[ebx + 24]        ; restore ebx
    pop ebx

    iretd                       ; return to thread
