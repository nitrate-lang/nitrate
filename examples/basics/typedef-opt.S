	.text
	.file	"typedef.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	movb	$10, -1(%rsp)
	movb	$20, -2(%rsp)
	movb	$30, -3(%rsp)
	movb	$30, -4(%rsp)
	movb	-4(%rsp), %al
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
