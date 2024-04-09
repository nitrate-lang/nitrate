	.text
	.file	"dereference.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	pushq	%rbx
	subq	$16, %rsp
	movl	$10000000, 8(%rsp)
	movq	8(%rsp), %r8
	leaq	.L_fa37JncCHr(%rip), %rdi
	leaq	8(%rsp), %rbx
	movl	$10000000, %esi
	movq	%rbx, %rdx
	movq	%rbx, %rcx
	xorl	%eax, %eax
	callq	printf@PLT
	movl	$20000000, 8(%rsp)
	movq	8(%rsp), %r8
	leaq	.L_yDsbzayy4c(%rip), %rdi
	movl	$20000000, %esi
	movq	%rbx, %rdx
	movq	%rbx, %rcx
	xorl	%eax, %eax
	callq	printf@PLT
	movl	$30000000, 8(%rsp)
	movq	8(%rsp), %r8
	leaq	.L_BWDxS22Jjz(%rip), %rdi
	movl	$30000000, %esi
	movq	%rbx, %rdx
	movq	%rbx, %rcx
	xorl	%eax, %eax
	callq	printf@PLT
	addq	$16, %rsp
	popq	%rbx
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.type	.L_fa37JncCHr,@object
	.data
	.p2align	4
.L_fa37JncCHr:
	.asciz	"x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\n"
	.size	.L_fa37JncCHr, 42

	.type	.L_yDsbzayy4c,@object
	.p2align	4
.L_yDsbzayy4c:
	.asciz	"x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\n"
	.size	.L_yDsbzayy4c, 42

	.type	.L_BWDxS22Jjz,@object
	.p2align	4
.L_BWDxS22Jjz:
	.asciz	"x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\n"
	.size	.L_BWDxS22Jjz, 42

	.section	".note.GNU-stack","",@progbits
