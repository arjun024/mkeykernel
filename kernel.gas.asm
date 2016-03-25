# Copyright (C) 2015  Xianguang Zhou
# License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html

.section .text

.align 4
.long 0x1BADB002
.long 0x00
.long - (0x1BADB002 + 0x00) 

.globl start
.globl keyboard_handler
.globl read_port
.globl write_port
.globl load_idt

.globl kmain 		
.globl keyboard_handler_main

read_port:
	mov 4(%esp),%edx
	in (%dx),%al	
	ret

write_port:
	mov 4(%esp),%edx
	mov 8(%esp),%al
	out %al,%dx
	ret

load_idt:
	mov 4(%esp),%edx
	lidtl (%edx)
	sti 				
	ret

keyboard_handler:                 
	call    keyboard_handler_main
	iret

start:
	cli 				
	mov $stack_space,%esp
	call kmain
	hlt 

.section .bss

.lcomm stack_range 8192
stack_space:
