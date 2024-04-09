	.text
	.file	"fn-stack-alloc.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	movb	$30, %al
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main

	.section	".note.GNU-stack","",@progbits
