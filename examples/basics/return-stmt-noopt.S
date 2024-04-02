	.text
	.file	"return-stmt.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	movb	$0, -1(%rsp)
	movb	-1(%rsp), %al
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
