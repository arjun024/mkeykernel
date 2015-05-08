
CFLAGS += -ffreestanding

SRCS := head.asm kernel.c
OBJS := $(foreach src,$(SRCS),$(basename $(src)).o)

LINK_SCRIPT := link.ld

KERN_BIN := kernel

all: $(KERN_BIN)

clean:
	rm $(KERN_BIN)
	rm $(OBJS)

$(KERN_BIN): $(OBJS)
	ld -m elf_i386 -T $(LINK_SCRIPT) -o $@ $(OBJS)

# Rule for compiling nasm assembly
%.o: %.asm
	nasm -f elf32 $< -o $@

# Rule for compiling C
%.o: %.c
	gcc -m32 $(CFLAGS) -c $< -o $@
