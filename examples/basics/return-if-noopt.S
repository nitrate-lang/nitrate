	.text
	.file	"return-if.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	movb	$1, %al
	testb	%al, %al
	je	.LBB0_2
	movb	$10, -1(%rsp)
.LBB0_2:
	movb	-1(%rsp), %al
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
