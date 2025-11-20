AS = nasm
CC = x86_64-elf-gcc
LD = x86_64-elf-ld

ASFLAGS = -f elf64
CFLAGS = -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
         -Wall -Wextra -O2 -I.
LDFLAGS = -n -T linker.ld

# Archivos objeto
ASM_OBJS = boot.o idt.o interrupts.o
C_OBJS = kernel.o terminal.o isr.o timer.o keyboard.o panic.o test_suite.o string.o \
		exception_test.o shell.o memory.o kmalloc.o memory_tests.o
OBJS = $(ASM_OBJS) $(C_OBJS)

all: os.iso

os.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# Reglas para archivos assembly
boot.o: boot.asm
	$(AS) $(ASFLAGS) -o $@ $<

idt.o: idt.asm
	$(AS) $(ASFLAGS) -o $@ $<

interrupts.o: interrupts.asm
	$(AS) $(ASFLAGS) -o $@ $<

# Reglas para archivos C
kernel.o: kernel.c terminal.h isr.h timer.h multiboot_header.h
	$(CC) $(CFLAGS) -c -o $@ $<

terminal.o: terminal.c terminal.h
	$(CC) $(CFLAGS) -c -o $@ $<

isr.o: isr.c isr.h terminal.h
	$(CC) $(CFLAGS) -c -o $@ $<

timer.o: timer.c timer.h terminal.h isr.h
	$(CC) $(CFLAGS) -c -o $@ $<

keyboard.o: keyboard.c keyboard.h terminal.h isr.h
	$(CC) $(CFLAGS) -c -o $@ $<

panic.o: panic.c panic.h terminal.h isr.h
	$(CC) $(CFLAGS) -c -o $@ $<

test_suite.o: test_suite.c test_suite.h terminal.h
	$(CC) $(CFLAGS) -c -o $@ $<

string.o: string.c string.h
	$(CC) $(CFLAGS) -c -o $@ $<	

exception_test.o: exception_test.c exception_test.h terminal.h
	$(CC) $(CFLAGS) -c -o $@ $<

shell.o: shell.c shell.h terminal.h panic.h string.h timer.h keyboard.h memory_tests.h
	$(CC) $(CFLAGS) -c -o $@ $<

memory.o: memory.c memory.h terminal.h multiboot_header.h string.h
	$(CC) $(CFLAGS) -c -o $@ $<

kmalloc.o: kmalloc.c kmalloc.h memory.h terminal.h string.h
	$(CC) $(CFLAGS) -c -o $@ $<

memory_tests.o: memory_tests.c memory_tests.h memory.h terminal.h kmalloc.h string.h test_suite.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Crear imagen ISO booteable
os.iso: os.bin
	mkdir -p isodir/boot/grub
	cp os.bin isodir/boot/os.bin
	echo 'set timeout=0' > isodir/boot/grub/grub.cfg
	echo 'set default=0' >> isodir/boot/grub/grub.cfg
	echo 'menuentry "MyOS" {' >> isodir/boot/grub/grub.cfg
	echo '    multiboot2 /boot/os.bin' >> isodir/boot/grub/grub.cfg
	echo '}' >> isodir/boot/grub/grub.cfg
	grub-mkrescue -o os.iso isodir

run: os.iso
	qemu-system-x86_64 -cdrom os.iso

debug: os.iso
	qemu-system-x86_64 -cdrom os.iso -d int -no-reboot -no-shutdown

clean:
	rm -f $(OBJS) os.bin os.iso
	rm -rf isodir

.PHONY: all clean run debug