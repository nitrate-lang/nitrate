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
	leaq	.L__unnamed_1(%rip), %rdi
	movl	$10000000, %esi
	movq	%rbx, %rdx
	movq	%rbx, %rcx
	movq	%rbx, %r8
	xorl	%eax, %eax
	callq	printf@PLT
	movl	$20000000, 4(%rsp)
	movq	8(%rsp), %rcx
	leaq	.L__unnamed_2(%rip), %rdi
	movl	$20000000, %esi
	movq	%rbx, %rdx
	movq	%rcx, %r8
	xorl	%eax, %eax
	callq	printf@PLT
	movq	8(%rsp), %rax
	movl	$30000000, (%rax)
	movl	4(%rsp), %esi
	movq	8(%rsp), %rcx
	leaq	.L__unnamed_3(%rip), %rdi
	movq	%rbx, %rdx
	movq	%rcx, %r8
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

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\n"
	.size	.L__unnamed_1, 42

	.type	.L__unnamed_2,@object
.L__unnamed_2:
	.asciz	"x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\n"
	.size	.L__unnamed_2, 42

	.type	.L__unnamed_3,@object
.L__unnamed_3:
	.asciz	"x = %d, &x = %p, x_ptr = %p, *x_ptr = %d\n"
	.size	.L__unnamed_3, 42

	.section	".note.GNU-stack","",@progbits
