	.text
	.file	"fn-params.q"
	.p2align	4, 0x90
	.type	.L_ZJ0f11CallMeFirst7f1c1c1p1p,@function
.L_ZJ0f11CallMeFirst7f1c1c1p1p:
	.cfi_startproc
	addb	$10, %dil
	movb	%dil, -1(%rsp)
	movb	-1(%rsp), %al
	retq
.Lfunc_end0:
	.size	.L_ZJ0f11CallMeFirst7f1c1c1p1p, .Lfunc_end0-.L_ZJ0f11CallMeFirst7f1c1c1p1p
	.cfi_endproc

	.p2align	4, 0x90
	.type	.L_ZJ0f12CallMeSecond7f1c1c1p1p,@function
.L_ZJ0f12CallMeSecond7f1c1c1p1p:
	.cfi_startproc
	shll	$2, %edi
	leal	(%rdi,%rdi,4), %eax
	movb	%al, -1(%rsp)
	movb	-1(%rsp), %al
	retq
.Lfunc_end1:
	.size	.L_ZJ0f12CallMeSecond7f1c1c1p1p, .Lfunc_end1-.L_ZJ0f12CallMeSecond7f1c1c1p1p
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
