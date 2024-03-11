    .extern main
    .extern __stack_end__
    .extern __bss_start__
    .extern __bss_end__

    .global _header

    .section .header, "axR"
_header:
    b	_entry
    nop
    # Reserve 8 words for args
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
    j       _bss
    addi    $8, $8, 4       # delay slot

_bss_done:
    # No need to copy .data
    # Setup stack, jump to C code
    la	$29, __stack_end__
    j	main
    nop
    .size   _entry, .-_entry
