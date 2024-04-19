	.text
	.file	"fn-call.q"
	.p2align	4, 0x90
	.type	.L_ZJ0f11CallMeFirst9f1c1a1c1p1p,@function
.L_ZJ0f11CallMeFirst9f1c1a1c1p1p:
	.cfi_startproc
	addb	$10, %dil
	movb	%dil, -1(%rsp)
	movb	-1(%rsp), %al
	retq
.Lfunc_end0:
	.size	.L_ZJ0f11CallMeFirst9f1c1a1c1p1p, .Lfunc_end0-.L_ZJ0f11CallMeFirst9f1c1a1c1p1p
	.cfi_endproc

	.p2align	4, 0x90
	.type	.L_ZJ0f12CallMeSecond9f1c1b1c1p1p,@function
.L_ZJ0f12CallMeSecond9f1c1b1c1p1p:
	.cfi_startproc
	shll	$2, %edi
	leal	(%rdi,%rdi,4), %eax
	movb	%al, -1(%rsp)
	movb	-1(%rsp), %al
	retq
.Lfunc_end1:
	.size	.L_ZJ0f12CallMeSecond9f1c1b1c1p1p, .Lfunc_end1-.L_ZJ0f12CallMeSecond9f1c1b1c1p1p
	.cfi_endproc

	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	$10, %edi
	callq	.L_ZJ0f11CallMeFirst9f1c1a1c1p1p
	movzbl	%al, %edi
	callq	.L_ZJ0f12CallMeSecond9f1c1b1c1p1p
	addb	$60, %al
	movb	%al, 7(%rsp)
	movb	7(%rsp), %al
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end2:
	.size	main, .Lfunc_end2-main
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
