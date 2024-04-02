	.text
	.file	"if-statement.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	movb	$1, %al
	testb	%al, %al
	je	.LBB0_5
	je	.LBB0_3
	movb	$10, -1(%rsp)
	movb	-1(%rsp), %al
	retq
.LBB0_5:
	xorl	%eax, %eax
	testb	%al, %al
	je	.LBB0_6
	movb	$40, -1(%rsp)
	movb	-1(%rsp), %al
	retq
.LBB0_3:
	movb	$20, -1(%rsp)
	movb	-1(%rsp), %al
	retq
.LBB0_6:
	movb	$50, -1(%rsp)
	movb	-1(%rsp), %al
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
