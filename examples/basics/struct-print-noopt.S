	.text
	.file	"struct-print.q"
	.p2align	4, 0x90
	.type	.L_ZJ0f12init__person66f1v6person52m49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p,@function
.L_ZJ0f12init__person66f1v6person52m49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p:
	.cfi_startproc
	leaq	.L__unnamed_1(%rip), %rax
	movq	%rax, 32(%rdi)
	movb	$30, 40(%rdi)
	leaq	.L__unnamed_2(%rip), %rax
	movq	%rax, (%rdi)
	leaq	.L__unnamed_3(%rip), %rax
	movq	%rax, 8(%rdi)
	leaq	.L__unnamed_4(%rip), %rax
	movq	%rax, 16(%rdi)
	leaq	.L__unnamed_5(%rip), %rax
	movq	%rax, 24(%rdi)
	retq
.Lfunc_end0:
	.size	.L_ZJ0f12init__person66f1v6person52m49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p, .Lfunc_end0-.L_ZJ0f12init__person66f1v6person52m49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p
	.cfi_endproc

	.p2align	4, 0x90
	.type	.L_ZJ0f13print__person63f1v6person49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p,@function
.L_ZJ0f13print__person63f1v6person49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset %rbx, -16
	movq	%rdi, %rbx
	movq	32(%rdi), %rsi
	leaq	.L__unnamed_6(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movzbl	40(%rbx), %esi
	leaq	.L__unnamed_7(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movq	(%rbx), %rsi
	leaq	.L__unnamed_8(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movq	8(%rbx), %rsi
	leaq	.L__unnamed_9(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movq	16(%rbx), %rsi
	leaq	.L__unnamed_10(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	movq	24(%rbx), %rsi
	leaq	.L__unnamed_11(%rip), %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end1:
	.size	.L_ZJ0f13print__person63f1v6person49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p, .Lfunc_end1-.L_ZJ0f13print__person63f1v6person49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p
	.cfi_endproc

	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	subq	$64, %rsp
	.cfi_def_cfa_offset 80
	.cfi_offset %rbx, -16
	leaq	16(%rsp), %rbx
	movq	%rbx, %rdi
	callq	.L_ZJ0f12init__person66f1v6person52m49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p
	movq	%rbx, %rdi
	callq	.L_ZJ0f13print__person63f1v6person49p46t6Person28m25t7Address3m1x3m1x3m1x3m1x3m1x3m1b1p1p
	leaq	.L__unnamed_12(%rip), %rdi
	movq	%rbx, %rsi
	xorl	%eax, %eax
	callq	printf@PLT
	movl	12(%rsp), %eax
	addq	$64, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end2:
	.size	main, .Lfunc_end2-main
	.cfi_endproc

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
