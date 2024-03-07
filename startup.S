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
	# Clear bss
	la	$8, __bss_start__
	la	$9, __bss_end__
	beq	$8, $9, _start
	nop
_bss:
	sw	$0, ($8)
	addi	$8, $8, 4
	bne	$8, $9, _bss
	nop

_start:
	# Setup stack, jump to C code
	la	$29, __stack_end__
	j	main
	nop
	.size   _entry, .-_entry
