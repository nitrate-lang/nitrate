	.text
	.file	"fn-stack-multi-alloc.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	movb	$6, -3(%rsp)
	movb	-2(%rsp), %al
	addb	-1(%rsp), %al
	addb	$6, %al
	movb	%al, -4(%rsp)
	movb	-4(%rsp), %al
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
