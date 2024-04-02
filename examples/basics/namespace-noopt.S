	.text
	.file	"namespace.q"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	movb	.L_ZJ0l18Number_CONSTANT__A1c0(%rip), %al
	movb	%al, -1(%rsp)
	movb	.L_ZJ0l18Number_CONSTANT__B1c0(%rip), %cl
	movb	%cl, -2(%rsp)
	addb	%cl, %al
	movb	%al, -3(%rsp)
	movb	-3(%rsp), %al
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.type	.L_ZJ0l18Number_CONSTANT__A1c0,@object
	.data
.L_ZJ0l18Number_CONSTANT__A1c0:
	.byte	10
	.size	.L_ZJ0l18Number_CONSTANT__A1c0, 1

	.type	.L_ZJ0l18Number_CONSTANT__B1c0,@object
.L_ZJ0l18Number_CONSTANT__B1c0:
	.byte	20
	.size	.L_ZJ0l18Number_CONSTANT__B1c0, 1

	.section	".note.GNU-stack","",@progbits
