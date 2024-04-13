	.text
	.file	"struct-member-access.q"
	.p2align	4, 0x90
	.type	.L_ZJ0f17get__person__info22f17t10PersonInfo1x1i1p1p,@function
.L_ZJ0f17get__person__info22f17t10PersonInfo1x1i1p1p:
	.cfi_startproc
	subq	$40, %rsp
	.cfi_def_cfa_offset 48
	movl	$100, %edi
	callq	malloc@PLT
	movq	%rax, 8(%rsp)
	leaq	.L__unnamed_1(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movq	8(%rsp), %rdi
	callq	gets@PLT
	leaq	.L__unnamed_2(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	leaq	16(%rsp), %rsi
	leaq	.L__unnamed_3(%rip), %rdi
	xorl	%eax, %eax
	callq	scanf@PLT
	movq	8(%rsp), %rax
	movl	16(%rsp), %edx
	movl	%edx, 32(%rsp)
	movq	%rax, 24(%rsp)
	addq	$40, %rsp
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
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	callq	.L_ZJ0f17get__person__info22f17t10PersonInfo1x1i1p1p
	movq	%rax, 8(%rsp)
	movl	%edx, 16(%rsp)
	leaq	.L__unnamed_4(%rip), %rdi
	movq	%rax, %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	movl	16(%rsp), %esi
	leaq	.L__unnamed_5(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movq	8(%rsp), %rdi
	callq	free@PLT
	addq	$24, %rsp
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
