	.text
	.file	"while.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	movb	$20, -1(%rsp)
	xorl	%eax, %eax
	.p2align	4, 0x90
.LBB0_1:
	testb	%al, %al
	jne	.LBB0_4
	cmpb	$21, -1(%rsp)
	jne	.LBB0_1
	movb	$20, -2(%rsp)
	movb	-2(%rsp), %al
	retq
.LBB0_4:
	movb	$60, -2(%rsp)
	movb	-2(%rsp), %al
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
