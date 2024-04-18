	.text
	.file	"struct-print.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	pushq	%r15
	pushq	%r14
	pushq	%r12
	pushq	%rbx
	subq	$56, %rsp
	leaq	.L__unnamed_1(%rip), %rsi
	movq	%rsi, 40(%rsp)
	movb	$30, 48(%rsp)
	leaq	.L__unnamed_2(%rip), %rbx
	movq	%rbx, 8(%rsp)
	leaq	.L__unnamed_3(%rip), %r12
	movq	%r12, 16(%rsp)
	leaq	.L__unnamed_4(%rip), %r15
	movq	%r15, 24(%rsp)
	leaq	.L__unnamed_5(%rip), %r14
	movq	%r14, 32(%rsp)
	leaq	.L__unnamed_6(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_7(%rip), %rdi
	movl	$30, %esi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_8(%rip), %rdi
	movq	%rbx, %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_9(%rip), %rdi
	movq	%r12, %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_10(%rip), %rdi
	movq	%r15, %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_11(%rip), %rdi
	movq	%r14, %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_12(%rip), %rdi
	leaq	8(%rsp), %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	addq	$56, %rsp
	popq	%rbx
	popq	%r12
	popq	%r14
	popq	%r15
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"John Doe"
	.size	.L__unnamed_1, 9

	.type	.L__unnamed_2,@object
.L__unnamed_2:
	.asciz	"123 Main St"
	.size	.L__unnamed_2, 12

	.type	.L__unnamed_3,@object
.L__unnamed_3:
	.asciz	"Springfield"
	.size	.L__unnamed_3, 12

	.type	.L__unnamed_4,@object
.L__unnamed_4:
	.asciz	"IL"
	.size	.L__unnamed_4, 3

	.type	.L__unnamed_5,@object
.L__unnamed_5:
	.asciz	"62701"
	.size	.L__unnamed_5, 6

	.type	.L__unnamed_6,@object
.L__unnamed_6:
	.asciz	"Name: %s\n"
	.size	.L__unnamed_6, 10

	.type	.L__unnamed_7,@object
.L__unnamed_7:
	.asciz	"Age: %d\n"
	.size	.L__unnamed_7, 9

	.type	.L__unnamed_8,@object
.L__unnamed_8:
	.asciz	"Address: %s\n"
	.size	.L__unnamed_8, 13

	.type	.L__unnamed_9,@object
.L__unnamed_9:
	.asciz	"City: %s\n"
	.size	.L__unnamed_9, 10

	.type	.L__unnamed_10,@object
.L__unnamed_10:
	.asciz	"State: %s\n"
	.size	.L__unnamed_10, 11

	.type	.L__unnamed_11,@object
.L__unnamed_11:
	.asciz	"Postal Code: %s\n"
	.size	.L__unnamed_11, 17

	.type	.L__unnamed_12,@object
.L__unnamed_12:
	.asciz	"&p: %p\n"
	.size	.L__unnamed_12, 8

	.section	".note.GNU-stack","",@progbits
