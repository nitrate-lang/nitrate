	.text
	.file	"for.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	movb	$20, -2(%rsp)
	.p2align	4, 0x90
.LBB0_1:
	cmpb	$100, -2(%rsp)
	jl	.LBB0_1
	movb	$60, -1(%rsp)
	movb	$60, %al
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
