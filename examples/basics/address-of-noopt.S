	.text
	.file	"address-of.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rax
	.cfi_def_cfa_offset 16
	movb	$10, 7(%rsp)
	leaq	.L_fa37JncCHr(%rip), %rdi
	leaq	7(%rsp), %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	popq	%rax
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.type	.L_fa37JncCHr,@object
	.data
	.p2align	4
.L_fa37JncCHr:
	.asciz	"address of x: %p\n"
	.size	.L_fa37JncCHr, 18

	.section	".note.GNU-stack","",@progbits
