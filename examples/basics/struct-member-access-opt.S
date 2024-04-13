	.text
	.file	"struct-member-access.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	pushq	%rbp
	pushq	%rbx
	subq	$24, %rsp
	movl	$100, %edi
	callq	malloc@PLT
	movq	%rax, %rbx
	movq	%rax, 8(%rsp)
	leaq	.L__unnamed_1(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movq	%rbx, %rdi
	callq	gets@PLT
	leaq	.L__unnamed_2(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	16(%rsp), %rsi
	leaq	.L__unnamed_3(%rip), %rdi
	xorl	%eax, %eax
	callq	scanf@PLT
	movq	8(%rsp), %rbx
	movl	16(%rsp), %ebp
	leaq	.L__unnamed_4(%rip), %rdi
	movq	%rbx, %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	.L__unnamed_5(%rip), %rdi
	movl	%ebp, %esi
	xorl	%eax, %eax
	callq	printf@PLT
	movq	%rbx, %rdi
	callq	free@PLT
	addq	$24, %rsp
	popq	%rbx
	popq	%rbp
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"Enter your name: "
	.size	.L__unnamed_1, 18

	.type	.L__unnamed_2,@object
.L__unnamed_2:
	.asciz	"Enter your age: "
	.size	.L__unnamed_2, 17

	.type	.L__unnamed_3,@object
.L__unnamed_3:
	.asciz	"%d"
	.size	.L__unnamed_3, 3

	.type	.L__unnamed_4,@object
.L__unnamed_4:
	.asciz	"Name: %s\n"
	.size	.L__unnamed_4, 10

	.type	.L__unnamed_5,@object
.L__unnamed_5:
	.asciz	"Age: %d\n"
	.size	.L__unnamed_5, 9

	.section	".note.GNU-stack","",@progbits
