	.text
	.file	"dereference.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	subq	$16, %rsp
	.cfi_def_cfa_offset 32
	.cfi_offset %rbx, -16
	movl	$10000000, 4(%rsp)
	leaq	4(%rsp), %rbx
	movq	%rbx, 8(%rsp)
	movq	4(%rsp), %r8
	leaq	.L_fa37JncCHr(%rip), %rdi
	movl	$10000000, %esi
	movq	%rbx, %rdx
	movq	%rbx, %rcx
	xorl	%eax, %eax
	callq	printf@PLT
	movl	$20000000, 4(%rsp)
	movq	8(%rsp), %rcx
	movq	(%rcx), %r8
	leaq	.L_yDsbzayy4c(%rip), %rdi
	movl	$20000000, %esi
	movq	%rbx, %rdx
	xorl	%eax, %eax
	callq	printf@PLT
	movq	8(%rsp), %rax
	movl	$30000000, (%rax)
	movl	4(%rsp), %esi
	movq	8(%rsp), %rcx
	movq	(%rcx), %r8
	leaq	.L_BWDxS22Jjz(%rip), %rdi
	movq	%rbx, %rdx
	xorl	%eax, %eax
	callq	printf@PLT
	addq	$16, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

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
