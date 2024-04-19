	.text
	.file	"struct-defaults.q"
	.p2align	4, 0x90
	.type	.L_ZJ0f13print__person44f1v37p34t6Person20m17t7Address1x1x1x1x1x1b1p1p,@function
.L_ZJ0f13print__person44f1v37p34t6Person20m17t7Address1x1x1x1x1x1b1p1p:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset %rbx, -16
	movq	%rdi, %rbx
	movq	32(%rdi), %rsi
	leaq	.L__unnamed_1(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movzbl	40(%rbx), %esi
	leaq	.L__unnamed_2(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movq	(%rbx), %rsi
	leaq	.L__unnamed_3(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movq	8(%rbx), %rsi
	leaq	.L__unnamed_4(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movq	16(%rbx), %rsi
	leaq	.L__unnamed_5(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movq	24(%rbx), %rsi
	leaq	.L__unnamed_6(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	.L_ZJ0f13print__person44f1v37p34t6Person20m17t7Address1x1x1x1x1x1b1p1p, .Lfunc_end0-.L_ZJ0f13print__person44f1v37p34t6Person20m17t7Address1x1x1x1x1x1b1p1p
	.cfi_endproc

	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	subq	$48, %rsp
	.cfi_def_cfa_offset 64
	.cfi_offset %rbx, -16
	movq	%rsp, %rbx
	movq	%rbx, %rdi
	callq	.L_ZJ0f13print__person44f1v37p34t6Person20m17t7Address1x1x1x1x1x1b1p1p
	leaq	.L__unnamed_7(%rip), %rdi
	movq	%rbx, %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	addq	$48, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
	.cfi_endproc

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
