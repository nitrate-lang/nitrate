	.text
	.file	"struct-defaults.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	subq	$56, %rsp
	leaq	.L__unnamed_1(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_2(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_3(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_4(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_5(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_6(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_7(%rip), %rdi
	leaq	8(%rsp), %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	addq	$56, %rsp
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"Name: %s\n"
	.size	.L__unnamed_1, 10

	.type	.L__unnamed_2,@object
.L__unnamed_2:
	.asciz	"Age: %d\n"
	.size	.L__unnamed_2, 9

	.type	.L__unnamed_3,@object
.L__unnamed_3:
	.asciz	"Address: %s\n"
	.size	.L__unnamed_3, 13

	.type	.L__unnamed_4,@object
.L__unnamed_4:
	.asciz	"City: %s\n"
	.size	.L__unnamed_4, 10

	.type	.L__unnamed_5,@object
.L__unnamed_5:
	.asciz	"State: %s\n"
	.size	.L__unnamed_5, 11

	.type	.L__unnamed_6,@object
.L__unnamed_6:
	.asciz	"Postal Code: %s\n"
	.size	.L__unnamed_6, 17

	.type	.L__unnamed_7,@object
.L__unnamed_7:
	.asciz	"&p: %p\n"
	.size	.L__unnamed_7, 8

	.section	".note.GNU-stack","",@progbits
