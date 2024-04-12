	.text
	.file	"struct-member-access.q"
	.p2align	4, 0x90
	.type	.L_ZJ0f17get__person__info22f17t10PersonInfo1x1b1p1p,@function
.L_ZJ0f17get__person__info22f17t10PersonInfo1x1b1p1p:
	.cfi_startproc
	subq	$56, %rsp
	.cfi_def_cfa_offset 64
	movl	16(%rsp), %esi
	movq	8(%rsp), %rax
	movq	%rax, 40(%rsp)
	movb	%sil, 48(%rsp)
	leaq	.L__unnamed_1(%rip), %rdi
	xorl	%eax, %eax
	callq	scanf@PLT
	movb	16(%rsp), %al
	movq	8(%rsp), %rcx
	movq	%rcx, 24(%rsp)
	movb	%al, 32(%rsp)
	movq	24(%rsp), %rax
	movb	32(%rsp), %dl
	addq	$56, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	.L_ZJ0f17get__person__info22f17t10PersonInfo1x1b1p1p, .Lfunc_end0-.L_ZJ0f17get__person__info22f17t10PersonInfo1x1b1p1p
	.cfi_endproc

	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	subq	$40, %rsp
	.cfi_def_cfa_offset 48
	callq	.L_ZJ0f17get__person__info22f17t10PersonInfo1x1b1p1p
	movq	%rax, 24(%rsp)
	movb	%dl, 32(%rsp)
	movq	%rax, 8(%rsp)
	movb	%dl, 16(%rsp)
	movzbl	%dl, %esi
	leaq	.L__unnamed_2(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	addq	$40, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
	.cfi_endproc

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
