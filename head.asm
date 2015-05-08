; Copyright (C) 2014  Arjun Sreedharan
; License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html

bits 32
section .text
        ;multiboot spec
        align 4
        dd 0x1BADB002              ;magic
        dd 0x00                    ;flags
        dd - (0x1BADB002 + 0x00)   ;checksum. m+f+c should be zero

global start
global keyboard_handler
global read_port
global write_port
global load_idt
global load_gdt

extern kmain 		;this is defined in the c file
extern keyboard_handler_main

read_port:
	mov edx, [esp + 4]
			;al is the lower 8 bits of eax
	in al, dx	;dx is the lower 16 bits of edx
	ret

write_port:
	mov   edx, [esp + 4]    
	mov   al, [esp + 4 + 4]  
	out   dx, al  
	ret

load_idt:
	mov edx, [esp + 4]
	lidt [edx]
	sti 				;turn on interrupts
	ret

load_gdt:
	mov edx, [esp + 4]
	lgdt [edx]
	ret

keyboard_handler:                 
	push ds
	push es
	push fs
	push gs
	pushad

	call    keyboard_handler_main

	popad
	pop ds
	pop es
	pop fs
	pop gs
	iretd

start:
	mov esp, stack_space
	call kmain
    cli
.loop:
	hlt 				;halt the CPU
    jmp .loop

section .bss
resb 8192; 8KB for stack
stack_space:
