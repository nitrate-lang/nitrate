	.text
	.file	"struct-member-access.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	pushq	%rax
	leaq	.L__unnamed_1(%rip), %rdi
	xorl	%eax, %eax
	callq	scanf@PLT
	leaq	.L__unnamed_2(%rip), %rdi
	xorl	%eax, %eax
	popq	%rcx
	jmp	printf@PLT
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"%d"
	.size	.L__unnamed_1, 3

	.type	.L__unnamed_2,@object
.L__unnamed_2:
	.asciz	"Age: %d\n"
	.size	.L__unnamed_2, 9

	.section	".note.GNU-stack","",@progbits
