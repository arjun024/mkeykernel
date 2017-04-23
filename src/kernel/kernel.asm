; Copyright (C) 2014  Arjun Sreedharan
; License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
; NASM syntax
bits 32
section .text
        ;multiboot spec
        align 4
        dd 0x1BADB002              ;magic
        dd 0x00                    ;flags
        dd - (0x1BADB002 + 0x00)   ;checksum. m+f+c should be zero

global start
global gdt_fush
global stack_space
global jump_usermode
global enter_usermode

extern kmain 		;this is defined in the c file
extern run_kshell

global loadPageDirectory
loadPageDirectory:
    push ebp
    mov ebp, esp
    mov eax, [esp + 8]
    mov cr3, eax
    mov esp, ebp
    pop ebp
    ret

global enablePaging
enablePaging:
    push ebp
    mov ebp, esp
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    mov esp, ebp
    pop ebp
    ret	

; This will set up our new segment registers. We need to do
; something special in order to set CS. We do what is called a
; far jump. A jump that includes a segment as well as an offset.
; This is declared in C as 'extern void gdt_flush();'
global gdt_flush     ; Allows the C code to link to this
extern gp            ; Says that '_gp' is in another file
gdt_flush:
    lgdt [gp]        ; Load the GDT with our '_gp' which is a special pointer
    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2   ; 0x08 is the offset to our code segment: Far jump!
flush2:
    ret               ; Returns back to the C code!

global tss_flush   ; Allows our C code to call tss_flush().
tss_flush:
   mov ax, 0x2B      ; Load the index of our TSS structure - The index is
                     ; 0x28, as it is the 5th selector and each is 8 bytes
                     ; long, but we set the bottom two bits (making 0x2B)
                     ; so that it has an RPL of 3, not zero.
   ltr ax            ; Load 0x2B into the task state register.
   ret   
    
    
 
global jump_usermode ;you may need to remove this _ to work right.. 
jump_usermode:
extern run_kshell
     mov ax,0x23
     mov ds,ax
     mov es,ax 
     mov fs,ax 
     mov gs,ax ;we don't need to worry about SS. it's handled by iret
 
     mov eax,esp
     push 0x23 ;user data segment with bottom 2 bits set for ring 3
     push eax ;push our current stack just for the heck of it
     pushf
     push 0x1B; ;user code segment with bottom 2 bits set for ring 3
     call run_kshell ;may need to remove the _ for this to work right 
     iret
 

global enter_usermode
enter_usermode:
    cli
    mov ax, 0x23	; user mode data selector is 0x20 (GDT entry 3). Also sets RPL to 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
; Now we can perform the switch to user mode. This is done by building the stack frame for IRET and issuing the IRET:
    push 0x23		; SS, notice it uses same selector as above
    push esp		; ESP
    pushfd			; EFLAGS
    push 0x1b		; CS, user mode code selector is 0x18. With RPL 3 this is 0x1b
    lea eax, [a]		; EIP first
    push eax

    call run_kshell
    iretd
a:
    add esp, 4 ; fix stack
    rts
 
 
start:
	cli 				;block interrupts
	mov esp, stack_space
	push ebx
	call kmain
	hlt 				;halt the CPU

section .bss
resb 32768; 8KB for stack
global stack_space
stack_space:
