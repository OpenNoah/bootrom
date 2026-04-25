    .extern main
    .extern __stack_end__
    .extern __bss_start__
    .extern __bss_end__
    .extern __data_load__
    .extern __data_start__
    .extern __data_end__

    .global _header

    .section .header, "axR"
_header:
    b	    _entry
    nop
    # Reserve some spaces for args
    .word	0
    .word	0
    .word	0
    .word	0
    .word	0
    .word	0
    .word	0
    # fw_args starts at offset 8
    .word	0
    .word	0
    .word	0
    .word	0
    .word	0
    .word	0
    .word	0
    .word	0
    # custom command args starts at offset 16
    .word	0
    .word	0
    .word	0
    .word	0
    .word	0
    .word	0
    .word	0
    .word	0
    .size   _header, .-_header

    .text
_entry:
    # Setup stack, jump to startup code
    la	$29, __stack_end__
    j	entry
    nop
    .size   _entry, .-_entry
