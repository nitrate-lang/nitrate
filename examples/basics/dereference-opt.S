	.text
	.file	"dereference.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	pushq	%r14
	pushq	%rbx
	pushq	%rax
	movl	$10000000, 4(%rsp)
	leaq	.L__unnamed_1(%rip), %r14
	leaq	4(%rsp), %rbx
	movq	%r14, %rdi
	movl	$10000000, %esi
	movq	%rbx, %rdx
	movq	%rbx, %rcx
	movq	%rbx, %r8
	xorl	%eax, %eax
	callq	printf@PLT
	movl	$20000000, 4(%rsp)
	movq	%r14, %rdi
	movl	$20000000, %esi
	movq	%rbx, %rdx
	movq	%rbx, %rcx
	movq	%rbx, %r8
	xorl	%eax, %eax
	callq	printf@PLT
	movl	$30000000, 4(%rsp)
	movq	%r14, %rdi
	movl	$30000000, %esi
	movq	%rbx, %rdx
	movq	%rbx, %rcx
	movq	%rbx, %r8
	xorl	%eax, %eax
	callq	printf@PLT
	addq	$8, %rsp
	popq	%rbx
	popq	%r14
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\n"
	.size	.L__unnamed_1, 42

	.section	".note.GNU-stack","",@progbits
