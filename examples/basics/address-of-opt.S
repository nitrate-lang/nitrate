	.text
	.file	"address-of.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	pushq	%rax
	movb	$10, 7(%rsp)
	leaq	.L__unnamed_1(%rip), %rdi
	leaq	7(%rsp), %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	popq	%rax
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"address of x: %p\n"
	.size	.L__unnamed_1, 18

	.section	".note.GNU-stack","",@progbits
