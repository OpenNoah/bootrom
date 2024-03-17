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
    b	_entry
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

    .section .text, "axR"
_entry:
    # Clear .bss
    la      $8, __bss_start__
    la      $9, __bss_end__
_bss:
    beq     $8, $9, _bss_done
    nop                     # delay slot
    sw      $0, ($8)
    addi    $8, $8, 4
    j       _bss
    nop                     # delay slot
_bss_done:

    # Load .data
    la      $8, __data_start__
    la      $9, __data_load__
    la      $10, __data_end__
_data:
    beq     $8, $10, _data_done
    nop                     # delay slot
    lw      $12, ($9)
    sw      $12, ($8)
    addi    $8, $8, 4
    addi    $9, $9, 4
    j       _data
    nop                     # delay slot
_data_done:

    # No need to copy .data
    # Setup stack, jump to C code
    la	$29, __stack_end__
    j	main
    nop
    .size   _entry, .-_entry
