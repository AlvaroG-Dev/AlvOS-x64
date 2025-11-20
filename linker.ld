/* linker.ld - Linker script para kernel 64-bit mejorado */
ENTRY(start)

SECTIONS
{
    /* El kernel se carga en 1MB - dirección física */
    . = 1M;

    /* Sección multiboot PRIMERO - crítica para GRUB */
    .multiboot : ALIGN(4K)
    {
        *(.multiboot)
    }

    .text : ALIGN(4K)
    {
        *(.text)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata)
    }

    .data : ALIGN(4K)
    {
        *(.data)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
        *(.stack)
    }

    /* Definir símbolos para el kernel */
    _kernel_start = .;
}