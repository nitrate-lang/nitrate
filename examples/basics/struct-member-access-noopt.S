	.text
	.file	"struct-member-access.q"
	.p2align	4, 0x90
	.type	.L_ZJ0f17get__person__info22f17t10PersonInfo1x1i1p1p,@function
.L_ZJ0f17get__person__info22f17t10PersonInfo1x1i1p1p:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	subq	$64, %rsp
	.cfi_def_cfa_offset 80
	.cfi_offset %rbx, -16
	leaq	8(%rsp), %rbx
	movl	$100, %edi
	callq	malloc@PLT
	movq	%rax, (%rsp)
	leaq	.L__unnamed_1(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movl	8(%rsp), %eax
	movq	(%rsp), %rdi
	movq	%rdi, 32(%rsp)
	movl	%eax, 40(%rsp)
	callq	gets@PLT
	leaq	.L__unnamed_2(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movl	8(%rsp), %eax
	movq	(%rsp), %rcx
	movq	%rcx, 16(%rsp)
	movl	%eax, 24(%rsp)
	leaq	.L__unnamed_3(%rip), %rdi
	movq	%rbx, %rsi
	xorl	%eax, %eax
	callq	scanf@PLT
	movq	(%rsp), %rax
	movl	8(%rsp), %edx
	movl	%edx, 56(%rsp)
	movq	%rax, 48(%rsp)
	addq	$64, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	.L_ZJ0f17get__person__info22f17t10PersonInfo1x1i1p1p, .Lfunc_end0-.L_ZJ0f17get__person__info22f17t10PersonInfo1x1i1p1p
	.cfi_endproc

	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	subq	$72, %rsp
	.cfi_def_cfa_offset 80
	callq	.L_ZJ0f17get__person__info22f17t10PersonInfo1x1i1p1p
	movq	%rax, 8(%rsp)
	movl	%edx, 16(%rsp)
	movq	%rax, 56(%rsp)
	movl	%edx, 64(%rsp)
	leaq	.L__unnamed_4(%rip), %rdi
	movq	%rax, %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	movl	16(%rsp), %esi
	movq	8(%rsp), %rax
	movq	%rax, 40(%rsp)
	movl	%esi, 48(%rsp)
	leaq	.L__unnamed_5(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movl	16(%rsp), %eax
	movq	8(%rsp), %rdi
	movq	%rdi, 24(%rsp)
	movl	%eax, 32(%rsp)
	callq	free@PLT
	addq	$72, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
	.cfi_endproc

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
